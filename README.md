**Pins, wiring and configuration**
------------------------------------------------------------
Component - Physical pin - CubeMX pin - CubeMX configuration
------------------------------------------------------------

Green LED        | D2  | PF15 | GPIO_Output
Yellow LED       | D4  | PF14 | GPIO_Output
Red LED          | D7  | PF13 | GPIO_Output
Blue LED         | D3  | PE13 | GPIO_Output
Green button     | D6  | PE9  | GPIO_Input
Yellow button    | D5  | PE11 | GPIO_Input
Red button       | A1  | PC0  | GPIO_Input
Blue button      | PD2*| PD2  | GPIO_Input
Piezo Buzzer     | D36 | PB10 | TIM2_CH3 PWM


**Ethernet notes**
We configured a direct Ethernet connection between a Windows PC and an STM32 NUCLEO-F767ZI microcontroller.
The PC was assigned the static IP address 10.128.0.1, while the STM32 was configured with 10.128.0.2. 
Both devices used the subnet mask 255.255.252.0 (/22), placing them on the same subnet.
We configured the Ethernet peripheral and the LwIP TCP/IP stack using STM32CubeMX. 
The firmware compiled and uploaded successfully using PlatformIO.

However, when we tested the connection using the ping command, the STM32 did not respond. 
The PC also failed to resolve the STM32's address through ARP.

One possible explanation is that the Ethernet hardware or PHY driver was not initialized correctly. 
Another possibility is that the LwIP stack was not processing incoming packets.
A particularly relevant issue is the structure of the main program loop. 
The Game_Run() function contains a blocking loop that waits for player input. 
As a result, the program may not call MX_LWIP_Process() frequently enough to handle network traffic.
Although this is a plausible software issue, the exact cause of the communication failure was not confirmed.

Because of schedule limitations, further investigation of the error was cancelled.  

