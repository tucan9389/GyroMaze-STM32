from com.arm.debug.dtsl.configurations import DTSLv1
from com.arm.debug.dtsl.components import FormatterMode
from com.arm.debug.dtsl.components import CSDAP
from com.arm.debug.dtsl.components import MemoryRouter
from com.arm.debug.dtsl.components import DapMemoryAccessor
from com.arm.debug.dtsl.components import Device
from com.arm.debug.dtsl.configurations.options import IIntegerOption
from com.arm.debug.dtsl.components import DSTREAMTraceCapture
from com.arm.debug.dtsl.components import ETMv3_4TraceSource
from com.arm.debug.dtsl.components import V7M_CSTPIU
from com.arm.debug.dtsl.components import ITMTraceSource

NUM_CORES_CORTEX_M3 = 1
DSTREAM_PORTWIDTH = 4

class M_Class_ETMv3_4(ETMv3_4TraceSource):
    def hasTriggers(self):
        return False
    
    def hasTraceStartPoints(self):
        return False
    
    def hasTraceStopPoints(self):
        return False
    
    def hasTraceRanges(self):
        return False
    


class DtslScript(DTSLv1):
    @staticmethod
    def getOptionList():
        return [
            DTSLv1.tabSet("options", "Options", childOptions=[
                DTSLv1.tabPage("trace", "Trace Capture", childOptions=[
                    DTSLv1.enumOption('traceCapture', 'Trace capture method', defaultValue="none",
                        values = [("none", "None"), ("DSTREAM", "DSTREAM 4GB Trace Buffer")],
                        setter=DtslScript.setTraceCaptureMethod),
                ]),
                DTSLv1.tabPage("cortexM3", "Cortex-M3", childOptions=[
                    DTSLv1.booleanOption('coreTrace', 'Enable Cortex-M3 core trace', defaultValue=False,
                        childOptions =
                            # Allow each source to be enabled/disabled individually
                            [ DTSLv1.booleanOption('Cortex_M3_%d' % c, "Enable Cortex-M3 %d trace" % c, defaultValue=True)
                            for c in range(0, NUM_CORES_CORTEX_M3) ]
                        ),
                ]),
                DTSLv1.tabPage("itm", "ITM", childOptions=[
                    DTSLv1.booleanOption('ITM0', 'Enable ITM 0 trace', defaultValue=False),
                ]),
            ])
        ]
    
    def __init__(self, root):
        DTSLv1.__init__(self, root)
        
        '''Do not add directly to this list - first check if the item you are adding is already present'''
        self.mgdPlatformDevs = []
        
        # Tracks which devices are managed when a trace mode is enabled
        self.mgdTraceDevs = {}
        
        # Locate devices on the platform and create corresponding objects
        self.discoverDevices()
        
        # Only DAP device is managed by default - others will be added when enabling trace, SMP etc
        if self.dap not in self.mgdPlatformDevs:
            self.mgdPlatformDevs.append(self.dap)
        
        self.exposeCores()
        
        traceComponentOrder = [ self.TPIU ]
        managedDevices = [ self.TPIU, self.DSTREAM ]
        self.setupDSTREAMTrace(DSTREAM_PORTWIDTH, traceComponentOrder, managedDevices)
        
        self.setManagedDeviceList(self.mgdPlatformDevs)
    
    # +----------------------------+
    # | Target dependent functions |
    # +----------------------------+
    
    def discoverDevices(self):
        '''Find and create devices'''
        
        self.dap = CSDAP(self, 2, "DAP")
        
        cortexM3coreDevs = [3]
        self.cortexM3cores = []
        
        streamID = 1
        
        etmDevs = [6]
        self.ETMs = []
        
        # ITM 0
        self.ITM0 = self.createITM(4, streamID, "ITM0")
        streamID += 1
        
        for i in range(0, NUM_CORES_CORTEX_M3):
            # Create core
            core = Device(self, cortexM3coreDevs[i], "Cortex-M3")
            self.cortexM3cores.append(core)
            
        for i in range(0, len(etmDevs)):
            # Create ETM
            etm = self.createETM(etmDevs[i], streamID, "ETMs[%d]" % i)
            streamID += 1
            
        # DSTREAM
        self.DSTREAM = DSTREAMTraceCapture(self, "DSTREAM")
        
        # TPIU
        self.TPIU = self.createTPIU(5, "TPIU")
        
    def exposeCores(self):
        for core in self.cortexM3cores:
            self.addDeviceInterface(self.createDAPWrapper(core))
    
    def setupDSTREAMTrace(self, portwidth, traceComponentOrder, managedDevices):
        '''Setup DSTREAM trace capture'''
        self.TPIU.setPortSize(portwidth)
        
        # Configure the DSTREAM for continuous trace
        self.DSTREAM.setTraceMode(DSTREAMTraceCapture.TraceMode.Continuous)
        self.DSTREAM.setPortWidth(portwidth)
        
        # Register other trace components
        self.DSTREAM.setTraceComponentOrder(traceComponentOrder)
        
        # Register the DSTREAM with the configuration
        self.addTraceCaptureInterface(self.DSTREAM)
        
        # Automatically handle connection/disconnection to trace components
        self.addManagedTraceDevices("DSTREAM", managedDevices)
    
    def getTMForCore(self, core):
        '''Get trace macrocell for core'''
        
        # Build map of cores to trace macrocells
        coreTMMap = {}
        coreTMMap[self.cortexM3cores[0]] = self.ETMs[0]
        
        return coreTMMap.get(core, None)
    
    def setTraceSourceEnabled(self, source, enabled):
        '''Enable/disable a trace source'''
        source.setEnabled(enabled)
    
    def createETM(self, etmDev, streamID, name):
        '''Create ETM of correct version'''
        if etmDev == 6:
            etm = M_Class_ETMv3_4(self, etmDev, streamID, name)
            # Disabled by default - will enable with option
            etm.setEnabled(False)
            self.ETMs.append(etm)
            return etm
    
    def createTPIU(self, tpiuDev, name):
        tpiu = V7M_CSTPIU(self, tpiuDev, name, self.cortexM3cores[0])
        # Disabled by default - will enable with option
        tpiu.setEnabled(False)
        return tpiu
    
    def setDSTREAMTraceEnabled(self, enabled):
        '''Enable/disable DSTREAM trace capture'''
        self.TPIU.setEnabled(enabled)
    
    def registerTraceSources(self, traceCapture):
        '''Register all trace sources with trace capture device'''
        for c in range(0, NUM_CORES_CORTEX_M3):
            coreTM = self.getTMForCore(self.cortexM3cores[c])
            if coreTM.isEnabled():
                self.registerCoreTraceSource(traceCapture, self.cortexM3cores[c], coreTM)
        
        self.registerTraceSource(traceCapture, self.ITM0)
    
    def registerCoreTraceSource(self, traceCapture, core, source):
        '''Register a trace source with trace capture device and enable triggers'''
        # Register with trace capture, associating with core
        traceCapture.addTraceSource(source, core.getID())
        
        # Source is managed by the configuration
        self.addManagedTraceDevices(traceCapture.getName(), [ source ])
    
    # +--------------------------------+
    # | Callback functions for options |
    # +--------------------------------+
    
    def optionValuesChanged(self):
        '''Callback to update the configuration state after options are changed'''
        if not self.isConnected():
            self.setInitialOptions()
        self.updateDynamicOptions()
        
    def setInitialOptions(self):
        '''Set the initial options'''
        
        traceMode = self.getOptionValue("options.trace.traceCapture")
        
        coreTraceEnabled = self.getOptionValue("options.cortexM3.coreTrace")
        for i in range(0, NUM_CORES_CORTEX_M3):
            thisCoreTraceEnabled = self.getOptionValue("options.cortexM3.coreTrace.Cortex_M3_%d" % i)
            enableSource = coreTraceEnabled and thisCoreTraceEnabled
            coreTM = self.getTMForCore(self.cortexM3cores[i])
            self.setTraceSourceEnabled(coreTM, enableSource)
        
        itmEnabled = self.getOptionValue("options.itm.ITM0")
        self.setTraceSourceEnabled(self.ITM0, itmEnabled)
        
        # Register trace sources for each trace sink
        self.registerTraceSources(self.DSTREAM)
        
        self.setManagedDeviceList(self.getManagedDevices(traceMode))
        
    def updateDynamicOptions(self):
        '''Update the dynamic options'''
        
    def getManagedDevices(self, traceKey):
        '''Get the required set of managed devices for this configuration'''
        deviceList = self.mgdPlatformDevs[:]
        for d in self.mgdTraceDevs.get(traceKey, []):
            if d not in deviceList:
                deviceList.append(d)
        
        return deviceList
    
    def setTraceCaptureMethod(self, method):
        if method == "none":
            self.setDSTREAMTraceEnabled(False)
        elif method == "DSTREAM":
            self.setDSTREAMTraceEnabled(True)
    
    def getETMs(self):
        '''Get the ETMs'''
        return self.ETMs
    
    # +------------------------------+
    # | Target independent functions |
    # +------------------------------+
    
    def registerTraceSource(self, traceCapture, source):
        '''Register trace source with trace capture device'''
        traceCapture.addTraceSource(source)
        self.addManagedTraceDevices(traceCapture.getName(), [ source ])
    
    def addManagedTraceDevices(self, traceKey, devs):
        '''Add devices to the set of devices managed by the configuration for this trace mode'''
        traceDevs = self.mgdTraceDevs.get(traceKey)
        if not traceDevs:
            traceDevs = []
            self.mgdTraceDevs[traceKey] = traceDevs
        for d in devs:
            if d not in traceDevs:
                traceDevs.append(d)
    
    def createDAPWrapper(self, core):
        '''Add a wrapper around a core to allow access to AHB and APB via the DAP'''
        return MemoryRouter(
            [DapMemoryAccessor("AHB", self.dap, 0, "AHB bus accessed via AP_0 on DAP_0"),
             DapMemoryAccessor("APB", self.dap, 1, "APB bus accessed via AP_1 on DAP_0")],
            core)
    
    def createITM(self, itmDev, streamID, name):
        itm = ITMTraceSource(self, itmDev, streamID, name)
        # Disabled by default - will enable with option
        itm.setEnabled(False)
        return itm
    
class DtslScript_RVI(DtslScript):
    @staticmethod
    def getOptionList():
        return [
            DTSLv1.tabSet("options", "Options", childOptions=[
            ])
        ]


