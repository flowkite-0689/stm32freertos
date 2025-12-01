// #include "rfid.h"
// #include "uart_dma.h"
// #include <stdio.h>
// #include <string.h>

// unsigned char *rfid_rx_buffer = NULL;
// unsigned char tx_buffer[RFID_FRAME_MAX_LEN] = {0};

// /**
//  * @brief RFID初始化
//  *
//  * @param rx_buffer  接收缓冲区
//  * @param len        缓冲区长度
//  */
// void RFID_dev_init(unsigned char *rx_buffer, unsigned short len)
// {
//     RFID_Init(rx_buffer, len);
//     rfid_rx_buffer = rx_buffer;
// }

// // BBC校验
// unsigned char BBC_Check(unsigned char *data, unsigned short len)
// {
//     unsigned char i, sum = 0;
//     for (i = 0; i < len; i++)
//     {
//         sum ^= data[i];
//     }
//     return ~sum;
// }

// /**
//  * @brief 发送一帧数据
//  *
//  * @param ctype  类型
//  * @param cmd    命令
//  * @param data  数据
//  * @param len   数据长度
//  */
// void RFID_SendFrame(unsigned char ctype, unsigned char cmd, unsigned char *data, unsigned char len)
// {
//     unsigned char i;
//     unsigned short tx_len = 0;
//     tx_buffer[tx_len++] = 6 + len;
//     tx_buffer[tx_len++] = ctype;
//     tx_buffer[tx_len++] = cmd;
//     tx_buffer[tx_len++] = len;
//     for (i = 0; i < len; i++)
//     {
//         tx_buffer[tx_len++] = data[i];
//     }
//     tx_buffer[tx_len++] = BBC_Check(tx_buffer, tx_len);
//     tx_buffer[tx_len++] = RFID_ETX;

//     RFID_SendBytes(tx_buffer, tx_len);
//     printf_array(tx_buffer, tx_len);
// }

// /**
//  * @brief 请求
//  *
//  * @return unsigned short  ATQ
//  */
// unsigned short RFID_Request(void)
// {
//     unsigned short len = 0;
//     unsigned char info = 0x52;
//     unsigned short ATQ = 0;
//     unsigned char timeout = RFID_TIMEOUT;
//     RFID_SendFrame(0x02, 0x41, &info, 0x01); // 请求
//     while (timeout--)
//     {
//         // 等待写卡完成
//         len = RFID_GetFrame_len();
//         if (len)
//         {
//             if (BBC_Check(rfid_rx_buffer, len - 2) == rfid_rx_buffer[len - 2]) // 接收成功
//             {
//                 ATQ = rfid_rx_buffer[4] | (rfid_rx_buffer[5] << 8);
//                 break;
//             }
//         }
//         IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//         RFID_delay_ms(1000);
//     }
//     return ATQ;
// }

// /**
//  * @brief 第一级防冲撞
//  *
//  * @return unsigned int  卡号uid
//  */
// unsigned int RFID_Anticollision(void)
// {
//     unsigned short len = 0;
//     unsigned short info = 0x0093;
//     unsigned int UID = 0;
//     unsigned char timeout = RFID_TIMEOUT;
//     RFID_SendFrame(0x02, 0x42, (unsigned char *)&info, 0x02); // 防冲撞
//     while (timeout--)
//     {
//         // 等待写卡完成
//         len = RFID_GetFrame_len();
//         if (len)
//         {
//             if (BBC_Check(rfid_rx_buffer, len - 2) == rfid_rx_buffer[len - 2]) // 接收成功
//             {
//                 UID = rfid_rx_buffer[4] | (rfid_rx_buffer[5] << 8) | (rfid_rx_buffer[6] << 16) | (rfid_rx_buffer[7] << 24);
//                 break;
//             }
//         }
//         IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//         RFID_delay_ms(1000);
//     }
//     return UID;
// }

// // 暂停
// unsigned char RFID_PiccHalt(void)
// {
//     unsigned char ret = 0;
//     unsigned short len = 0;
//     unsigned short info = 0;
//     unsigned char timeout = RFID_TIMEOUT;
//     RFID_SendFrame(0x02, 0x04, (unsigned char *)&info, 0x00); // 防冲撞
//     while (timeout--)
//     {
//         // 等待写卡完成
//         len = RFID_GetFrame_len();
//         if (len)
//         {
//             if (BBC_Check(rfid_rx_buffer, len - 2) == rfid_rx_buffer[len - 2]) // 接收成功
//             {
//                 ret = 1;
//                 break;
//             }
//         }
//         IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//         RFID_delay_ms(1000);
//     }
//     return ret;
// }

// /**
//  * @brief 第一级防碰撞选择卡
//  *
//  * @param UID  卡号
//  * @return unsigned short  SAK
//  */
// unsigned short RFID_SelectCard(unsigned int UID)
// {
//     unsigned short len = 0;
//     unsigned char timeout = RFID_TIMEOUT;
//     unsigned char info[0x05] = {0};
//     unsigned char SAK = 0;

//     info[0] = 0x93;
//     // 低字节在前
//     info[1] = UID & 0xFF;
//     info[2] = (UID >> 8) & 0xFF;
//     info[3] = (UID >> 16) & 0xFF;
//     info[4] = (UID >> 24) & 0xFF;
//     RFID_SendFrame(0x02, 0x43, info, 0x05);

//     while (timeout--)
//     {
//         len = RFID_GetFrame_len();
//         if (len)
//         {
//             printf("选卡打印：");
//             printf_array(rfid_rx_buffer, len);
//             // 检查响应格式
//             if (BBC_Check(rfid_rx_buffer, len - 2) == rfid_rx_buffer[len - 2])
//             {
//                 SAK = rfid_rx_buffer[8]; // 修正SAK位置，应该是第9个字节（索引8）
//                 break;
//             }
//         }
//         IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//         RFID_delay_ms(1000);
//     }
//     return SAK;
// }

// /**
//  * @brief 验证密码
//  *
//  * @param keyAB 密钥A-0x60 / 密钥B-0x61
//  * @param UID   卡号
//  * @param key    密钥 (6个字节,默认密钥为0xFFFFFFFFFFFF)
//  * @param block  块号 (S50: 0-63)
//  * @return unsigned char
//  */
// unsigned char RFID_Authentication(unsigned char keyAB, unsigned int UID, unsigned char *key, unsigned char block)
// {
//     unsigned short len = 0;
//     unsigned char timeout = RFID_TIMEOUT;
//     unsigned char info[0x0C] = {0};
//     unsigned char status = 0;

//     info[0] = keyAB;
//     info[1] = UID & 0xFF;
//     info[2] = (UID >> 8) & 0xFF;
//     info[3] = (UID >> 16) & 0xFF;
//     info[4] = (UID >> 24) & 0xFF;
//     memcpy(&info[5], key, 6);
//     info[11] = block;

//     RFID_SendFrame(0x02, 0x46, info, 0x0C);

//     while (timeout--)
//     {
//         len = RFID_GetFrame_len();
//         if (len)
//         {
//             printf("验证密码响应：");
//             printf_array(rfid_rx_buffer, len);

//             if (BBC_Check(rfid_rx_buffer, len - 2) == rfid_rx_buffer[len - 2])
//             {
//                 // 检查验证状态，通常0x00表示成功，0x0e表示卡验证失败
//                 if (rfid_rx_buffer[4] == 0x00) {
//                     status = 1;
//                     printf("验证密码成功\n");
//                 } else {
//                     printf("验证密码失败，状态码: 0x%x\n", rfid_rx_buffer[4]);
//                 }
//                 break;
//             }
//         }
//         IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//         RFID_delay_ms(1000);
//     }
//     return status;
// }

// /**
//  * @brief 读卡
//  *
//  * @param block 块号 (S50: 0-63)
//  * @param recv  接收数据
//  * @param recvlen  接收数据长度(16个字节)
//  * @return unsigned char
//  */
// unsigned char RFID_ReadCard(unsigned char block, unsigned char *recv, unsigned short recvlen)
// {
//     unsigned short len = 0;
//     unsigned char timeout = RFID_TIMEOUT;
//     unsigned char info = block;
//     unsigned char status = 0;
//     RFID_SendFrame(0x02, 0x47, &info, 0x01); // 读卡
//     while (timeout--)
//     {
//         len = RFID_GetFrame_len();
//         if (len)
//         {
//             if (BBC_Check(rfid_rx_buffer, len - 2) == rfid_rx_buffer[len - 2]) // 接收成功
//             {
//                 memcpy(recv, rfid_rx_buffer + 4, recvlen);
//                 status = 1; // 验证成功
//                 break;
//             }
//         }
//         IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//         RFID_delay_ms(1000);
//     }
//     return status;
// }

// /**
//  * @brief 写卡
//  *
//  * @param block 块号 (S50: 0-63)
//  * @param send  发送数据
//  * @param sendlen  发送数据长度(16个字节)
//  * @return unsigned short
//  */
// unsigned char RFID_WriteCard(unsigned char block, unsigned char *send, unsigned short sendlen)
// {
//     unsigned short len = 0;
//     unsigned char timeout = RFID_TIMEOUT;
//     unsigned char info[0x11] = {0};
//     unsigned char status = 0;

//     info[0] = block;
//     memcpy(info + 1, send, sendlen);

//     RFID_SendFrame(0x02, 0x48, info, 0x11);
//     while (timeout--)
//     {
//         len = RFID_GetFrame_len();
//         if (len)
//         {
//             printf("写卡响应：");
//             printf_array(rfid_rx_buffer, len);

//             if (len >= 5 && rfid_rx_buffer[1] == 0x02 && rfid_rx_buffer[2] == 0x48)
//             {
//                 if (BBC_Check(rfid_rx_buffer, len - 2) == rfid_rx_buffer[len - 2])
//                 {
//                     // 检查写卡结果
//                     if (rfid_rx_buffer[4] == 0x00)
//                     { // 通常0x00表示成功
//                         status = 1;
//                         printf("写卡成功\n");
//                     }
//                     else
//                     {
//                         printf("写卡失败，错误码: 0x%x\n", rfid_rx_buffer[4]);
//                     }
//                     break;
//                 }
//             }
//         }
//         IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//         RFID_delay_ms(1000);
//     }
//     return status;
// }
// void RFID_demo(void)
// {
//     unsigned char rx_buffer[50];
//     RFID_dev_init(rx_buffer, sizeof(rx_buffer));

//     printf("=== RFID测试开始 ===\n");
//     IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
    
//     // 请求
//     printf("1. 请求...\n");
//     IWDG_ReloadCounter(); // 在长时间操作前喂狗
//     unsigned short ATQ = RFID_Request();
//     if (ATQ != S50_ATQ)
//     {
//         printf("请求失败, ATQ=%#x\n", ATQ);
//         return;
//     }
//     printf("ATQ=%#x\n", ATQ);
//     delay_ms(500);
//     IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//     // 防碰撞
//     printf("2. 防碰撞...\n");
//     IWDG_ReloadCounter(); // 在长时间操作前喂狗
//     unsigned int UID = RFID_Anticollision();
//     if (!UID)
//     {
//         printf("防碰撞失败\n");
//         return;
//     }
//     printf("UID=%#x\n", UID);
//     delay_ms(500);
//     IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//     // 选择卡
//     printf("3. 选择卡...\n");
//     IWDG_ReloadCounter(); // 在长时间操作前喂狗
//     unsigned short SAK = RFID_SelectCard(UID);
//     printf("SAK=%#x\n", SAK);
//     if (SAK != S50_SAK)
//     {
//         printf("选择卡失败，期望SAK=%#x，实际SAK=%#x\n", S50_SAK, SAK);
//         // 继续尝试，有些卡片可能SAK不同
//     }
//     delay_ms(500);
//     IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//     // 验证密码
//     printf("4. 验证密码...\n");
//     IWDG_ReloadCounter(); // 在长时间操作前喂狗
//     unsigned char key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//     if (!RFID_Authentication(0x61, UID, key, 62))
//     {
//         printf("验证密码失败\n");
//         return;
//     }
//     delay_ms(500);
//     IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//     // 写卡
//     printf("5. 写卡...\n");
//     IWDG_ReloadCounter(); // 在长时间操作前喂狗
//     unsigned char send[16] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
//                               0x9, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
//     if (!RFID_WriteCard(62, send, 16))
//     {
//         printf("写卡失败\n");
//         // 继续尝试读卡
//     }
//     delay_ms(500);
//     IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//     // 读卡
//     printf("6. 读卡...\n");
//     IWDG_ReloadCounter(); // 在长时间操作前喂狗
//     unsigned char recv[16] = {0};
//     if (!RFID_ReadCard(62, recv, 16))
//     {
//         printf("读卡失败\n");
//         return;
//     }
//     printf("读卡成功:\n");
//     printf_array(recv, 16);
//     IWDG_ReloadCounter(); // 喂狗，防止看门狗复位
//     printf("=== RFID测试结束 ===\n");
// }
