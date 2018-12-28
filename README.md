# GyroMaze-STM32
이 프로젝트는 임베디드 실험의 텀 프로젝트 결과물입니다. 궁금하신 사항이 있으면 이슈에 남겨주세요! 질문과 제안은 언제나 환영입니다.

![gyromaze demo 002](resource/demo-001.gif)

## 개요

모바일 기기로 미로판을 조작하는 간단한 구슬 미로 게임입니다. 

## 실험 환경

### 실험 장비

- STM32(Cortex-M3)
- JTAG(Joint Test Action Group)
- DSTREAM
- Android Device

### 소프트웨어

- Eclipse for DS-5
- Android Studio

## 하드웨어

### 사용 모듈

| Servo Motor 1                     | Servo Motor 2                                                | Light Sensor                                                 | Bluetooth Module                            | LED                                                          |
| --------------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------- | ------------------------------------------------------------ |
| ![servo1](resource/servo1.jpg)[1] | ![servo2](resource/servo2.jpg)[[2]](https://goo.gl/images/vFNxJ6) | [![lightsensor](resource/lightsensor.jpeg)[3]](https://goo.gl/images/EHFuvN) | ![bluetooth module](resource/bluetooth.png) | ![led](resource/led.png)[[4]](https://smartstore.naver.com/young-je/products/253019772) |
| TSR11965                          | TF0006SER                                                    | 광센서 - 포토레지스터(찾는중..)                              | FB755AC                                     | 고휘도 LED전구 3                                             |

### 구성도

(준비중)

### 회로도

![모듈과 브레드보드](resource/modules_breadboard.png)

### 모듈별 사진

#### 미로판 제어 지점

|                        위에서 본 모습                        |                      대각선에서 본 모습                      |
| :----------------------------------------------------------: | :----------------------------------------------------------: |
| ![397636A6-112D-4B83-853E-200A53D8185B](resource/397636A6-112D-4B83-853E-200A53D8185B.JPG) | ![02542460-0DDD-4341-888B-A4FD266E6D54](resource/02542460-0DDD-4341-888B-A4FD266E6D54.JPG) |

#### 게임 시작 지점

|               입구에 설치된 자동문 - 열렸을 때               |               입구에 설치된 자동문 - 닫혔을 때               |
| :----------------------------------------------------------: | :----------------------------------------------------------: |
| ![9EF86831-0276-433F-A960-36CD2DF37754](resource/9EF86831-0276-433F-A960-36CD2DF37754.JPG) | ![C623DCD5-6099-4872-8271-E882BE166C12](resource/C623DCD5-6099-4872-8271-E882BE166C12.JPG) |

#### 게임 끝 지점

|              게임 끝 지점 - 구슬이 들어왔을 때               |                게임 끝 지점 - 구슬이 없을 때                 |
| :----------------------------------------------------------: | :----------------------------------------------------------: |
| ![12E8D8A7-CE0D-47B7-BEF6-C37DAC57F290](resource/12E8D8A7-CE0D-47B7-BEF6-C37DAC57F290.JPG) | ![ECF4DE6E-2FA1-4772-B9B6-166E88DD01FA](resource/ECF4DE6E-2FA1-4772-B9B6-166E88DD01FA.JPG) |

## 소프트웨어

### 소스파일

- [`gyro3.c`](ens7_20/gyro3.c)
- [`bluetooth.h`](ens7_20/bluetooth.h), [`bluetooth.c`](ens7_20/bluetooth.c)
- [`servo.h`](ens7_20/servo.h), [`servo.c`](ens7_20/servo.c)
- [`lightsensor.h`](ens7_20/lightsensor.h), [`lightsensor.c`](ens7_20/lightsensor.c)
- [`queue.h`](ens7_20/queue.h), [`queue.c`](ens7_20/queue.c)

### 게임 로직

(준비중)

## 미로 제작

(준비중)

## 시연

![DEMO-002](resource/DEMO-002.gif)

### 연관 프로젝트

- [star51/GyroBT](https://github.com/star51/GyroBT) - STM32를 제어하는데 사용된 블루투스 기반의 안드로이드 프로젝트

### 참여자

- 강주연
- 정성훈
- 허태준
- 곽도영

## 참고

[1] 서보모터 1 사진, 

[2] 서보모터 2 사진, https://goo.gl/images/vFNxJ6

[3] 조도센서 사진, https://goo.gl/images/EHFuvN

[4] LED 사진, https://smartstore.naver.com/young-je/products/253019772