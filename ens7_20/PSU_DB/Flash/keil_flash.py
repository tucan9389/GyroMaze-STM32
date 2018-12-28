from flashprogrammer.flash_method_v1 import FlashMethodv1
from com.arm.debug.flashprogrammer import FlashProgrammerRuntimeException
from com.arm.debug.flashprogrammer.IProgress import OperationResult
from com.arm.debug.flashprogrammer import FlashRegion
from com.arm.debug.flashprogrammer import TargetStatus

import java.io.File
import java.lang.Exception

import os.path

from flashprogrammer.flmfile import FlmFileReader
from flashprogrammer.device import ensureDeviceOpen
from flashprogrammer.execution import ensureDeviceStopped, funcCall, funcCallRepeat, getWorkingRAMRequired
from flashprogrammer.image_loader import loadAllCodeSegmentsToTarget, getStaticBase
from flashprogrammer.device_memory import writeToTarget, readFromTarget, compareBuffers


FUNC_ERASE   = 1
FUNC_PROGRAM = 2
FUNC_VERIFY  = 3

MAX_REPEATS = 20

def str2bool(string):
    return string and string.lower() in ("true", "yes", "1", "ok")

class KeilFlash(FlashMethodv1):
    def __init__(self, methodServices):
        FlashMethodv1.__init__(self, methodServices)

        algorithmFile = self.locateFile(self.getParameter('algorithm'))
        self.flmReader = FlmFileReader(algorithmFile)
        self.flashInfo = self.flmReader.getFlashDeviceInfo()


    def getDefaultRegions(self):
        addr = self.flashInfo['address']
        size = self.flashInfo['size']

        return [ FlashRegion(addr, size) ]


    def getDefaultParameters(self):
        params = {}
        for k, v in self.flashInfo.items():
            params[k] = self.flmReader.formatFlashDeviceInfoEntry(k, v)
        return params


    def setup(self):
        # connect to core & stop
        self.conn = self.getConnection()
        coreName = self.getParameter("coreName")
        if coreName is None:
            raise FlashProgrammerRuntimeException, "coreName not specified in flash configuration file"
        self.dev = self.conn.getDeviceInterfaces().get(coreName)
        if self.dev is None:
            raise FlashProgrammerRuntimeException, "Cannot find core %s.  Please check your flash configuration file" % coreName
        self.deviceOpened = ensureDeviceOpen(self.dev)
        ensureDeviceStopped(self.dev)

        # get properties of working memory
        self.ramAddr = int(self.getParameter("ramAddress"), 0)
        self.ramSize = int(self.getParameter("ramSize"), 0)
        nextAddr = self.ramAddr

        # determine code / data layout
        #   code first
        low, high = self.flmReader.getLoadSegmentsAddressRange()
        self.algoAddr = self.ramAddr
        self.algoLoadOffset = self.algoAddr - low
        self.algoSize = high - low
        nextAddr += self.algoSize

        #   working RAM for function execution
        self.execWorkingRam = nextAddr
        nextAddr += getWorkingRAMRequired()

        #   then stack
        self.stackSize = 0x100
        self.stackBottom = nextAddr
        self.stackTop = self.stackBottom + self.stackSize - 4
        nextAddr += self.stackSize

        #   remaining memory is write buffer
        self.writeBufferAddr = nextAddr
        self.writeBufferSize = self.ramAddr - nextAddr + self.ramSize

        self.debug("Loading algorithm to %08x [%08x], working RAM: %08x stack: %08x..%08x, writeBuffer: %08x [%08x]" % (
                   self.algoAddr, self.algoSize,
                   self.execWorkingRam,
                   self.stackBottom, self.stackTop,
                   self.writeBufferAddr, self.writeBufferSize)
                   )

        # load algorithm to target
        loadAllCodeSegmentsToTarget(self.dev, self.flmReader, self.algoAddr)
        self.staticBase = getStaticBase(self.flmReader, self.algoAddr)

        self.pageSize = int(self.getParameter("programPageSize"), 0)

        self.clockFrequency = 0
        if "clockFrequency" in self.getParameters():
            self.clockFrequency = int(self.getParameter("clockFrequency"), 0)


    def getFuncAddr(self, funcName):
        funcInfo = self.flmReader.getFunctionInfo()[funcName]
        funcAddr = funcInfo['address']
        if funcInfo['thumb']:
            funcAddr |= 1
        funcAddr += self.algoLoadOffset

        return funcAddr


    def setStaticBase(self, kwargs):
        if not self.staticBase is None:
            # create parameter dict if needed
            if kwargs is None:
                kwargs = {}
            kwargs['staticBase'] = self.staticBase
        return kwargs


    def callFunction(self, funcName, *args, **kwargs):
        kwargs = self.setStaticBase(kwargs)
        return funcCall(self.dev, self.getFuncAddr(funcName), args, self.stackTop, **kwargs)


    def callFunctionRepeat(self, funcName, argLists, **kwargs):
        kwargs = self.setStaticBase(kwargs)
        return funcCallRepeat(self.dev, self.getFuncAddr(funcName), argLists,
                              self.stackTop, self.execWorkingRam,
                              **kwargs)


    def teardown(self):
        if self.deviceOpened:
            # close device connection if opened by this script
            self.dev.closeConn()

        # registers and memory have been clobbered
        return TargetStatus.STATE_LOST


    def program(self, regionID, offset, data):
        region = self.getRegion(regionID)
        addr = region.getAddress() + offset

        # perform verification unless verify parameter is set to false
        hasVerify = True
        if 'verify' in self.getParameters():
            hasVerify = str2bool(self.getParameter('verify'))

        # relative percentages of operations
        ERASE_SIZE = 40
        PROGRAM_SIZE = 50
        VERIFY_SIZE = 10

        TOTAL_SIZE = ERASE_SIZE + PROGRAM_SIZE
        if hasVerify:
            TOTAL_SIZE += VERIFY_SIZE

        progress = self.operationStarted(
            'Programming 0x%x bytes to 0x%08x' % (data.getSize(), addr),
            TOTAL_SIZE)

        try:

            if not self.isCancelled():
                self.doErase(addr, data.getSize(), self.subOperation(progress, 'Erasing', data.getSize(), ERASE_SIZE))
                progress.progress('Erasing completed', ERASE_SIZE)

            if not self.isCancelled():
                self.doWrite(addr, data, self.subOperation(progress, 'Writing', data.getSize(), PROGRAM_SIZE))
                progress.progress('Writing completed', ERASE_SIZE+PROGRAM_SIZE)

            # optionally verify
            if hasVerify and not self.isCancelled():
                self.doVerify(addr, data, self.subOperation(progress, 'Verifying', data.getSize(), VERIFY_SIZE))
                progress.progress('Verifying completed', ERASE_SIZE+PROGRAM_SIZE+VERIFY_SIZE)

            if self.isCancelled():
                progress.completed(OperationResult.CANCELLED, 'Programming cancelled')
            else:
                progress.completed(OperationResult.SUCCESS, 'Programming completed')
        except (Exception, java.lang.Exception), e:
            # exceptions may be derived from Java Exception or Python Exception
            progress.completed(OperationResult.FAILURE, 'Programming failed')
            raise

        # registers and memory have been clobbered
        return TargetStatus.STATE_LOST
    

    def __getTimeoutInSeconds(self, timeoutName):
        '''Convert timeout to ms, if timeout is -ve (or None) return None; if it
        is 0 return 0; if it is +ve convert to a fraction of a second'''
        
        if str2bool(self.getParameter("disableTimeouts")):
            return None
        
        timeoutInMs = int(self.getParameter(timeoutName))
                              
        if (timeoutInMs <= 0) or (timeoutInMs == None):
            return None
        
        try:
            return timeoutInMs / 1000.0
        except:
            return None

    def doErase(self, addr, size, progress):
        sectorEraseTimeout = self.__getTimeoutInSeconds('eraseSectorTimeout')

        result = (OperationResult.SUCCESS, 'Erasing completed')
        try:
            if self.callFunction('Init', addr, self.clockFrequency, FUNC_ERASE) != 0:
                raise FlashProgrammerRuntimeException, "Failed to initialise for erasing"

            # Erasing requires running the EraseSector routine on the target
            # for each page to be erased.  As this will involve many short operations,
            # it can be speeded up by writing the arguments for several calls into
            # memory, then calling a routine that calls EraseSector for each

            # build argument lists & expected return values
            nextAddr = addr
            endAddr = addr + size
            argLists = []
            while nextAddr < endAddr:
                # each call takes address to be erased and is expected to return 0
                argLists.append( ( [ nextAddr ], 0 ) )
                nextAddr += self.flmReader.getOffsetToNextSector(addr)

            # call EraseSector repeatedly with arg lists
            #   break the calls into batches to provide progress reporting and
            #   limit the amount of target memory required for arguments
            for i in range(0, len(argLists), MAX_REPEATS):
                if self.isCancelled():
                    result = (OperationResult.CANCELLED, 'Cancelled')
                    break

                # run one batch
                batchArgs = argLists[i:i+MAX_REPEATS]
                opsComplete, lastRet = self.callFunctionRepeat('EraseSector', batchArgs, timeout=sectorEraseTimeout*len(batchArgs))
                if opsComplete < len(batchArgs):
                    failAddr = argLists[opsComplete][0][0]
                    raise FlashProgrammerRuntimeException, "Failed to erase at %08x" % failAddr

                # report progress
                lastAddrErased = batchArgs[-1][0][0]
                bytesErased = lastAddrErased + self.flmReader.getOffsetToNextSector(lastAddrErased) - addr
                progress.progress('Erased 0x%x bytes' % bytesErased, bytesErased)

            if self.callFunction('UnInit', FUNC_ERASE) != 0:
                raise FlashProgrammerRuntimeException, "Failed to uninitialise after erasing"

        except (Exception, java.lang.Exception), e:
            # exceptions may be derived from Java Exception or Python Exception
            result = (OperationResult.FAILURE, 'Erasing failed')
            raise

        finally:
            progress.completed(*result)


    def doWrite(self, addr, data, progress):
        result = (OperationResult.SUCCESS, 'Writing completed')
        programPageTimeout = self.__getTimeoutInSeconds('programPageTimeout')

        try:
            if self.callFunction('Init', addr, self.clockFrequency, FUNC_PROGRAM) != 0:
                raise FlashProgrammerRuntimeException, "Failed to initialise for erasing"

            # write as many pages as can fit into write buffer
            # ensuring that a partial page is used if page size is > write buffer
            pagesPerWrite = max(1, self.writeBufferSize / self.pageSize)
            writeSize = self.pageSize * pagesPerWrite

            data.seek(0)
            bytesWritten = 0
            while bytesWritten < data.getSize():
                if self.isCancelled():
                    result = (OperationResult.CANCELLED, 'Cancelled')
                    break

                # get next block of data
                buf = data.getData(writeSize)

                # write data to download buffer
                writeToTarget(self.dev, self.writeBufferAddr, buf)

                # Programming requires running the ProgramPage routine on the target
                # for each page to be erased.  As this will involve many short operations,
                # it can be speeded up by writing the arguments for several calls into
                # memory, then calling a routine that calls ProgramPage for each

                # build argument lists to call ProgramPage for each page in this block
                argLists = []
                bufAddr = self.writeBufferAddr
                bytesLeftToProgram = len(buf)
                while bytesLeftToProgram > 0:
                    bytesThisPage = min(self.pageSize, bytesLeftToProgram)
                    # each call takes address to be programmed, the number of bytes and the source address
                    # and is expected to return 0
                    argLists.append( ( [ addr, bytesThisPage, bufAddr ], 0 ) )
                    bytesLeftToProgram -= bytesThisPage
                    addr += bytesThisPage
                    bufAddr += bytesThisPage

                # call ProgramPage repeatedly with arg lists
                #   break the calls into batches to provide progress reporting and
                #   limit the amount of target memory required for arguments
                for i in range(0, len(argLists), MAX_REPEATS):
                    if self.isCancelled():
                        result = (OperationResult.CANCELLED, 'Cancelled')
                        break

                    # run one batch
                    passArgs = argLists[i:i+MAX_REPEATS]
                    opsComplete, lastRet = self.callFunctionRepeat('ProgramPage', passArgs, timeout=programPageTimeout*len(passArgs))
                    if opsComplete < len(passArgs):
                        failArgs = argLists[opsComplete][0]
                        raise FlashProgrammerRuntimeException, "Failed to program %x bytes at %08x: %d" % (failArgs[1], failArgs[0], lastRet)

                    # report progress
                    bytesThisPass = sum(args[1] for args, expRet in passArgs)
                    bytesWritten += bytesThisPass
                    progress.progress('Written 0x%x bytes' % bytesWritten, bytesWritten)

            if self.callFunction('UnInit', FUNC_PROGRAM) != 0:
                raise FlashProgrammerRuntimeException, "Failed to uninitialise after programming"

        except (Exception, java.lang.Exception), e:
            # exceptions may be derived from Java Exception or Python Exception
            result = (OperationResult.FAILURE, 'Writing failed')
            raise

        finally:
            progress.completed(*result)


    def doVerify(self, addr, data, progress):
        if 'Verify' in self.flmReader.getFunctionInfo():
            # Use algorithm to verify if it provides it
            self.doVerifyByAlgorithm(addr, data, progress)
        else:
            # Otherwise attempt to verify by reading back
            self.doVerifyByRead(addr, data, progress)


    def doVerifyByAlgorithm(self, addr, data, progress):
        result = (OperationResult.SUCCESS, 'Verifying completed')

        try:
            if self.callFunction('Init', addr, self.clockFrequency, FUNC_VERIFY) != 0:
                raise FlashProgrammerRuntimeException, "Failed to initialise for erasing"

            # write as many pages as can fit into write buffer
            # ensuring that a partial page is used if page size is > write buffer
            pagesPerWrite = max(1, self.writeBufferSize / self.pageSize)
            writeSize = self.pageSize * pagesPerWrite

            data.seek(0)
            bytesVerified = 0
            while bytesVerified < data.getSize():
                if self.isCancelled():
                    result = (OperationResult.CANCELLED, 'Cancelled')
                    break

                # get next block of data
                buf = data.getData(writeSize)

                # write data to buffer
                writeToTarget(self.dev, self.writeBufferAddr, buf)

                # call verify func for each page
                bufAddr = self.writeBufferAddr
                bytesLeftToVerify = len(buf)
                while bytesLeftToVerify > 0:
                    if self.isCancelled():
                        break

                    bytesThisPage = min(self.pageSize, bytesLeftToVerify)

                    # Verify() returns addr+sz on success, address of failure on error
                    res = self.callFunction('Verify', addr, bytesThisPage, bufAddr)
                    if res != addr+bytesThisPage:
                        raise FlashProgrammerRuntimeException, "Failed to verify at address %08x" % res

                    bytesLeftToVerify -= bytesThisPage
                    addr += bytesThisPage
                    bufAddr += bytesThisPage

                    bytesVerified += bytesThisPage
                    progress.progress('Verified %d bytes' % bytesVerified, bytesVerified)

            if self.callFunction('UnInit', FUNC_VERIFY) != 0:
                raise FlashProgrammerRuntimeException, "Failed to uninitialise after programming"

        except (Exception, java.lang.Exception), e:
            # exceptions may be derived from Java Exception or Python Exception
            result = (OperationResult.FAILURE, 'Verifying failed: ' + e.getMessage())
            raise

        finally:
            progress.completed(*result)


    def doVerifyByRead(self, addr, data, progress):
        result = (OperationResult.SUCCESS, 'Verifying completed')

        data.seek(0)
        bytesVerified = 0
        try:
            while bytesVerified < data.getSize():
                if self.isCancelled():
                    result = (OperationResult.CANCELLED, 'Cancelled')
                    break

                # get next block of data
                buf = data.getData(self.pageSize)

                # read back from target
                readBuf = readFromTarget(self.dev, addr, len(buf))

                # compare contents
                res = compareBuffers(buf, readBuf)
                if res != len(buf):
                    raise FlashProgrammerRuntimeException, "Verify failed at address: %08x" % (addr + res)

                bytesVerified += len(buf)
                addr += len(buf)
                progress.progress('Verified %d bytes' % bytesVerified, bytesVerified)

        except (Exception, java.lang.Exception), e:
            # exceptions may be derived from Java Exception or Python Exception
            result = (OperationResult.FAILURE, 'Verifying failed: ' + e.getMessage())
            raise

        finally:
            progress.completed(*result)
