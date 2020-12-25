/******************************************************************************
 *
 *  Copyright (C) 2009-2012 Realtek Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  Filename:      userial_vendor.c
 *
 *  Description:   Contains vendor-specific userial functions
 *
 ******************************************************************************/
#undef NDEBUG
#define LOG_TAG "bt_userial_vendor"

#include <utils/Log.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/eventfd.h>
#include "userial.h"
#include "userial_vendor.h"

#define RTK_SINGLE_CMD_EVENT
/******************************************************************************
**  Constants & Macros
******************************************************************************/

#ifndef VNDUSERIAL_DBG
#define VNDUSERIAL_DBG TRUE
#endif

#if (VNDUSERIAL_DBG == TRUE)
#define VNDUSERIALDBG(param, ...) {ALOGD(param, ## __VA_ARGS__);}
#else
#define VNDUSERIALDBG(param, ...) {}
#endif

#define VND_PORT_NAME_MAXLEN    256

#ifndef BT_CHIP_HW_FLOW_CTRL_ON
#define BT_CHIP_HW_FLOW_CTRL_ON TRUE
#endif

/******************************************************************************
**  Extern functions
******************************************************************************/
extern char rtkbt_transtype;

/******************************************************************************
**  Local type definitions
******************************************************************************/
#if !defined(EFD_SEMAPHORE)
#  define EFD_SEMAPHORE (1 << 0)
#endif

#define RTK_DATA_RECEIVED 1
#define RTK_DATA_SEND     0
struct rtk_object_t {
  int fd;                              // the file descriptor to monitor for events.
  void *context;                       // a context that's passed back to the *_ready functions..
  pthread_mutex_t lock;                // protects the lifetime of this object and all variables.

  void (*read_ready)(void *context);   // function to call when the file descriptor becomes readable.
  void (*write_ready)(void *context);  // function to call when the file descriptor becomes writeable.
};

/* vendor serial control block */
typedef struct
{
    int fd;                     /* fd to Bluetooth device */
    int uart_fd[2];
    int epoll_fd;
    int cpoll_fd;
    int event_fd;
    struct termios termios;     /* serial terminal of BT port */
    char port_name[VND_PORT_NAME_MAXLEN];
    pthread_t thread_socket_id;
    pthread_t thread_uart_id;
    pthread_t thread_coex_id;
    bool thread_running;

    RTB_QUEUE_HEAD *recv_data;
    RTB_QUEUE_HEAD *send_data;
    RTB_QUEUE_HEAD *data_order;
} vnd_userial_cb_t;

#define RTK_NO_INTR(fn)  do {} while ((fn) == -1 && errno == EINTR)

/******************************************************************************
**  Static functions
******************************************************************************/
static void h5_data_ready_cb(serial_data_type_t type, unsigned int total_length);
static uint16_t h5_int_transmit_data_cb(serial_data_type_t type, uint8_t *data, uint16_t length) ;

/******************************************************************************
**  Static variables
******************************************************************************/
static vnd_userial_cb_t vnd_userial;
static const hci_h5_t* h5_int_interface;
static int packet_recv_state = RTKBT_PACKET_IDLE;
static unsigned int packet_bytes_need = 0;
static serial_data_type_t current_type = 0;
static struct rtk_object_t rtk_socket_object;
static struct rtk_object_t rtk_coex_object;
static unsigned char h4_read_buffer[2048] = {0};
static int h4_read_length = 0;

static int coex_packet_recv_state = RTKBT_PACKET_IDLE;
static int coex_packet_bytes_need = 0;
static serial_data_type_t coex_current_type = 0;
static unsigned char coex_resvered_buffer[2048] = {0};
static int coex_resvered_length = 0;

#ifdef RTK_SINGLE_CMD_EVENT
static int cmd_packet_recv_state = RTKBT_PACKET_IDLE;
static int cmd_packet_bytes_need = 0;
static serial_data_type_t cmd_current_type = 0;
static unsigned char cmd_resvered_header[10] = {0};
static int cmd_resvered_length = 0;
#endif

static rtk_parse_manager_t * rtk_parse_manager = NULL;

static  hci_h5_callbacks_t h5_int_callbacks = {
    .h5_int_transmit_data_cb = h5_int_transmit_data_cb,
    .h5_data_ready_cb = h5_data_ready_cb,
};

/*****************************************************************************
**   Helper Functions
*****************************************************************************/

/*******************************************************************************
**
** Function        userial_to_tcio_baud
**
** Description     helper function converts USERIAL baud rates into TCIO
**                  conforming baud rates
**
** Returns         TRUE/FALSE
**
*******************************************************************************/
uint8_t userial_to_tcio_baud(uint8_t cfg_baud, uint32_t *baud)
{
    if (cfg_baud == USERIAL_BAUD_115200)
        *baud = B115200;
    else if (cfg_baud == USERIAL_BAUD_4M)
        *baud = B4000000;
    else if (cfg_baud == USERIAL_BAUD_3M)
        *baud = B3000000;
    else if (cfg_baud == USERIAL_BAUD_2M)
        *baud = B2000000;
    else if (cfg_baud == USERIAL_BAUD_1M)
        *baud = B1000000;
    else if (cfg_baud == USERIAL_BAUD_1_5M)
        *baud = B1500000;
    else if (cfg_baud == USERIAL_BAUD_921600)
        *baud = B921600;
    else if (cfg_baud == USERIAL_BAUD_460800)
        *baud = B460800;
    else if (cfg_baud == USERIAL_BAUD_230400)
        *baud = B230400;
    else if (cfg_baud == USERIAL_BAUD_57600)
        *baud = B57600;
    else if (cfg_baud == USERIAL_BAUD_19200)
        *baud = B19200;
    else if (cfg_baud == USERIAL_BAUD_9600)
        *baud = B9600;
    else if (cfg_baud == USERIAL_BAUD_1200)
        *baud = B1200;
    else if (cfg_baud == USERIAL_BAUD_600)
        *baud = B600;
    else
    {
        ALOGE( "userial vendor open: unsupported baud idx %i", cfg_baud);
        *baud = B115200;
        return FALSE;
    }

    return TRUE;
}

#if (BT_WAKE_VIA_USERIAL_IOCTL==TRUE)
/*******************************************************************************
**
** Function        userial_ioctl_init_bt_wake
**
** Description     helper function to set the open state of the bt_wake if ioctl
**                  is used. it should not hurt in the rfkill case but it might
**                  be better to compile it out.
**
** Returns         none
**
*******************************************************************************/
void userial_ioctl_init_bt_wake(int fd)
{
    uint32_t bt_wake_state;

    /* assert BT_WAKE through ioctl */
    ioctl(fd, USERIAL_IOCTL_BT_WAKE_ASSERT, NULL);
    ioctl(fd, USERIAL_IOCTL_BT_WAKE_GET_ST, &bt_wake_state);
    VNDUSERIALDBG("userial_ioctl_init_bt_wake read back BT_WAKE state=%i", \
               bt_wake_state);
}
#endif // (BT_WAKE_VIA_USERIAL_IOCTL==TRUE)


/*****************************************************************************
**   Userial Vendor API Functions
*****************************************************************************/

/*******************************************************************************
**
** Function        userial_vendor_init
**
** Description     Initialize userial vendor-specific control block
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_init(char *bt_device_node)
{
    vnd_userial.fd = -1;
    char value[100];
    snprintf(vnd_userial.port_name, VND_PORT_NAME_MAXLEN, "%s", \
            bt_device_node);
    if(rtkbt_transtype & RTKBT_TRANS_H5) {
        h5_int_interface = hci_get_h5_int_interface();
        h5_int_interface->h5_int_init(&h5_int_callbacks);
    }
    rtk_parse_manager = NULL;
    property_get("persist.bluetooth.rtkcoex", value, "true");
    if(strncmp(value, "true", 4) == 0) {
        rtk_parse_manager = rtk_parse_manager_get_interface();
        rtk_parse_manager->rtk_parse_init();
    }
    vnd_userial.data_order = RtbQueueInit();
    vnd_userial.recv_data = RtbQueueInit();
    vnd_userial.send_data = RtbQueueInit();
}


/*******************************************************************************
**
** Function        userial_vendor_open
**
** Description     Open the serial port with the given configuration
**
** Returns         device fd
**
*******************************************************************************/
int userial_vendor_open(tUSERIAL_CFG *p_cfg)
{
    uint32_t baud;
    uint8_t data_bits;
    uint16_t parity;
    uint8_t stop_bits;

    vnd_userial.fd = -1;

    if (!userial_to_tcio_baud(p_cfg->baud, &baud))
    {
        return -1;
    }

    if(p_cfg->fmt & USERIAL_DATABITS_8)
        data_bits = CS8;
    else if(p_cfg->fmt & USERIAL_DATABITS_7)
        data_bits = CS7;
    else if(p_cfg->fmt & USERIAL_DATABITS_6)
        data_bits = CS6;
    else if(p_cfg->fmt & USERIAL_DATABITS_5)
        data_bits = CS5;
    else
    {
        ALOGE("userial vendor open: unsupported data bits");
        return -1;
    }

    if(p_cfg->fmt & USERIAL_PARITY_NONE)
        parity = 0;
    else if(p_cfg->fmt & USERIAL_PARITY_EVEN)
        parity = PARENB;
    else if(p_cfg->fmt & USERIAL_PARITY_ODD)
        parity = (PARENB | PARODD);
    else
    {
        ALOGE("userial vendor open: unsupported parity bit mode");
        return -1;
    }

    if(p_cfg->fmt & USERIAL_STOPBITS_1)
        stop_bits = 0;
    else if(p_cfg->fmt & USERIAL_STOPBITS_2)
        stop_bits = CSTOPB;
    else
    {
        ALOGE("userial vendor open: unsupported stop bits");
        return -1;
    }

    ALOGI("userial vendor open: opening %s", vnd_userial.port_name);

    if ((vnd_userial.fd = open(vnd_userial.port_name, O_RDWR)) == -1)
    {
        ALOGE("userial vendor open: unable to open %s", vnd_userial.port_name);
        return -1;
    }

    tcflush(vnd_userial.fd, TCIOFLUSH);

    tcgetattr(vnd_userial.fd, &vnd_userial.termios);
    cfmakeraw(&vnd_userial.termios);

    if(p_cfg->hw_fctrl == USERIAL_HW_FLOW_CTRL_ON)
    {
        ALOGI("userial vendor open: with HW flowctrl ON");
        vnd_userial.termios.c_cflag |= (CRTSCTS | stop_bits| parity);
    }
    else
    {
        ALOGI("userial vendor open: with HW flowctrl OFF");
        vnd_userial.termios.c_cflag &= ~CRTSCTS;
        vnd_userial.termios.c_cflag |= (stop_bits| parity);

    }

    tcsetattr(vnd_userial.fd, TCSANOW, &vnd_userial.termios);
    tcflush(vnd_userial.fd, TCIOFLUSH);

    tcsetattr(vnd_userial.fd, TCSANOW, &vnd_userial.termios);
    tcflush(vnd_userial.fd, TCIOFLUSH);
    tcflush(vnd_userial.fd, TCIOFLUSH);

    /* set input/output baudrate */
    cfsetospeed(&vnd_userial.termios, baud);
    cfsetispeed(&vnd_userial.termios, baud);
    tcsetattr(vnd_userial.fd, TCSANOW, &vnd_userial.termios);


#if (BT_WAKE_VIA_USERIAL_IOCTL==TRUE)
    userial_ioctl_init_bt_wake(vnd_userial.fd);
#endif

    ALOGI("device fd = %d open", vnd_userial.fd);

    return vnd_userial.fd;
}

static void userial_socket_close(void)
{
    int result;

    if ((result = close(vnd_userial.uart_fd[0])) < 0)
        ALOGE( "%s (fd:%d) FAILED result:%d", __func__, vnd_userial.uart_fd[0], result);

    if (epoll_ctl(vnd_userial.epoll_fd, EPOLL_CTL_DEL, vnd_userial.uart_fd[1], NULL) == -1)
      ALOGE("%s unable to unregister fd %d from epoll set: %s", __func__, vnd_userial.uart_fd[1], strerror(errno));

    if ((result = close(vnd_userial.uart_fd[1])) < 0)
        ALOGE( "%s (fd:%d) FAILED result:%d", __func__, vnd_userial.uart_fd[1], result);

    pthread_join(vnd_userial.thread_socket_id, NULL);
    close(vnd_userial.epoll_fd);
    vnd_userial.epoll_fd = -1;
    vnd_userial.uart_fd[0] = -1;
    vnd_userial.uart_fd[1] = -1;
}

static void userial_uart_close(void)
{
    int result;
    if ((result = close(vnd_userial.fd)) < 0)
        ALOGE( "%s (fd:%d) FAILED result:%d", __func__, vnd_userial.fd, result);
    pthread_join(vnd_userial.thread_uart_id, NULL);
}

static void userial_coex_close(void)
{
    int result;

    if (epoll_ctl(vnd_userial.cpoll_fd, EPOLL_CTL_DEL, vnd_userial.event_fd, NULL) == -1)
      ALOGE("%s unable to unregister fd %d from epoll set: %s", __func__, vnd_userial.event_fd, strerror(errno));

    if ((result = close(vnd_userial.event_fd)) < 0)
        ALOGE( "%s (fd:%d) FAILED result:%d", __func__, vnd_userial.event_fd, result);

    close(vnd_userial.cpoll_fd);
    pthread_join(vnd_userial.thread_coex_id, NULL);
    vnd_userial.cpoll_fd = -1;
    vnd_userial.event_fd = -1;
}


/*******************************************************************************
**
** Function        userial_vendor_close
**
** Description     Conduct vendor-specific close work
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_close(void)
{
    int result;

    if (vnd_userial.fd == -1)
        return;



    vnd_userial.thread_running = false;

    userial_socket_close();
    userial_uart_close();
    userial_coex_close();

    vnd_userial.fd = -1;
    if(rtk_parse_manager) {
        rtk_parse_manager->rtk_parse_cleanup();
    }
    rtk_parse_manager = NULL;


	if((rtkbt_transtype & RTKBT_TRANS_UART) && (rtkbt_transtype & RTKBT_TRANS_H5)) {
#if (BT_WAKE_VIA_USERIAL_IOCTL==TRUE)
        /* de-assert bt_wake BEFORE closing port */
        ioctl(vnd_userial.fd, USERIAL_IOCTL_BT_WAKE_DEASSERT, NULL);
#endif
        h5_int_interface->h5_int_cleanup();

    }
}

/*******************************************************************************
**
** Function        userial_vendor_set_baud
**
** Description     Set new baud rate
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_set_baud(uint8_t userial_baud)
{
    uint32_t tcio_baud;
    ALOGI("userial_vendor_set_baud");
    userial_to_tcio_baud(userial_baud, &tcio_baud);

    if(cfsetospeed(&vnd_userial.termios, tcio_baud)<0)
        ALOGE("cfsetospeed fail");

    if(cfsetispeed(&vnd_userial.termios, tcio_baud)<0)
        ALOGE("cfsetispeed fail");

    if(tcsetattr(vnd_userial.fd, TCSANOW, &vnd_userial.termios)<0)
        ALOGE("tcsetattr fail ");

    tcflush(vnd_userial.fd, TCIOFLUSH);
}

/*******************************************************************************
**
** Function        userial_vendor_ioctl
**
** Description     ioctl inteface
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_ioctl(userial_vendor_ioctl_op_t op, void *p_data)
{
    switch(op)
    {
#if (BT_WAKE_VIA_USERIAL_IOCTL==TRUE)
        case USERIAL_OP_ASSERT_BT_WAKE:
            VNDUSERIALDBG("## userial_vendor_ioctl: Asserting BT_Wake ##");
            ioctl(vnd_userial.fd, USERIAL_IOCTL_BT_WAKE_ASSERT, NULL);
            break;

        case USERIAL_OP_DEASSERT_BT_WAKE:
            VNDUSERIALDBG("## userial_vendor_ioctl: De-asserting BT_Wake ##");
            ioctl(vnd_userial.fd, USERIAL_IOCTL_BT_WAKE_DEASSERT, NULL);
            break;

        case USERIAL_OP_GET_BT_WAKE_STATE:
            ioctl(vnd_userial.fd, USERIAL_IOCTL_BT_WAKE_GET_ST, p_data);
            break;
#endif  //  (BT_WAKE_VIA_USERIAL_IOCTL==TRUE)

        default:
            break;
    }
}

/*******************************************************************************
**
** Function        userial_set_port
**
** Description     Configure UART port name
**
** Returns         0 : Success
**                 Otherwise : Fail
**
*******************************************************************************/
int userial_set_port(char *p_conf_name, char *p_conf_value, int param)
{
    strcpy(vnd_userial.port_name, p_conf_value);

    return 0;
}

/*******************************************************************************
**
** Function        userial_vendor_set_hw_fctrl
**
** Description     Conduct vendor-specific close work
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_set_hw_fctrl(uint8_t hw_fctrl)
{
    struct termios termios_old;

    if (vnd_userial.fd == -1)
    {
        ALOGE("vnd_userial.fd is -1");
        return;
    }

    tcgetattr(vnd_userial.fd, &termios_old);
    if(hw_fctrl)
    {
        if(termios_old.c_cflag & CRTSCTS)
        {
            BTVNDDBG("userial_vendor_set_hw_fctrl already hw flowcontrol on");
            return;
        }
        else
        {
            termios_old.c_cflag |= CRTSCTS;
            tcsetattr(vnd_userial.fd, TCSANOW, &termios_old);
            BTVNDDBG("userial_vendor_set_hw_fctrl set hw flowcontrol on");
        }
    }
    else
    {
        if(termios_old.c_cflag & CRTSCTS)
        {
            termios_old.c_cflag &= ~CRTSCTS;
            tcsetattr(vnd_userial.fd, TCSANOW, &termios_old);
            return;
        }
        else
        {
            ALOGI("userial_vendor_set_hw_fctrl set hw flowcontrol off");
            return;
        }
    }
}

static uint16_t h4_int_transmit_data(uint8_t *data, uint16_t total_length) {
    assert(data != NULL);
    assert(total_length > 0);

    uint16_t length = total_length;
    uint16_t transmitted_length = 0;
    while (length > 0) {
        ssize_t ret = write(vnd_userial.fd, data + transmitted_length, length);
        switch (ret) {
            case -1:
            ALOGE("In %s, error writing to the uart serial port: %s", __func__, strerror(errno));
            goto done;
        case 0:
            // If we wrote nothing, don't loop more because we
            // can't go to infinity or beyond, ohterwise H5 can resend data
            ALOGE("%s, ret %d", __func__, ret);
            goto done;
        default:
            transmitted_length += ret;
            length -= ret;
            break;
        }
    }

done:;
    return transmitted_length;
}

static void userial_enqueue_coex_rawdata(unsigned char * buffer, int length, bool is_recved)
{
    RTK_BUFFER* skb_data = RtbAllocate(length, 0);
    RTK_BUFFER* skb_type = RtbAllocate(1, 0);
    memcpy(skb_data->Data, buffer, length);
    skb_data->Length = length;
    if(is_recved) {
        *skb_type->Data = RTK_DATA_RECEIVED;
        skb_type->Length = 1;
        RtbQueueTail(vnd_userial.recv_data, skb_data);
        RtbQueueTail(vnd_userial.data_order, skb_type);
    }
    else {
        *skb_type->Data = RTK_DATA_SEND;
        skb_type->Length = 1;
        RtbQueueTail(vnd_userial.send_data, skb_data);
        RtbQueueTail(vnd_userial.data_order, skb_type);
    }

    if (eventfd_write(vnd_userial.event_fd, 1) == -1) {
        ALOGE("%s unable to write for coex event fd.", __func__);
    }
}

static int userial_coex_recv_data_handler(unsigned char * recv_buffer, int total_length)
{
    serial_data_type_t type = 0;
    unsigned char * p_data = recv_buffer;
    int length = total_length;
    HC_BT_HDR * p_buf;
    uint8_t boundary_flag;
    uint16_t len, handle, acl_length, l2cap_length;
    switch (coex_packet_recv_state) {
        case RTKBT_PACKET_IDLE:
            coex_packet_bytes_need = 1;
            while(length) {
                type = p_data[0];
                length--;
                p_data++;
                assert((type > DATA_TYPE_COMMAND) && (type <= DATA_TYPE_EVENT));
                if (type < DATA_TYPE_ACL || type > DATA_TYPE_EVENT) {
                    ALOGE("%s invalid data type: %d", __func__, type);
                    if(!length)
                        return total_length;

                    continue;
                }
                break;
            }
            coex_current_type = type;
            coex_packet_recv_state = RTKBT_PACKET_TYPE;
            //fall through

        case RTKBT_PACKET_TYPE:
            if(coex_current_type == DATA_TYPE_ACL) {
                coex_packet_bytes_need = 4;
            }
            else if(coex_current_type == DATA_TYPE_EVENT) {
                coex_packet_bytes_need = 2;
            }
            else {
                coex_packet_bytes_need = 3;
            }
            coex_resvered_length = 0;
            coex_packet_recv_state = RTKBT_PACKET_HEADER;
            //fall through

        case RTKBT_PACKET_HEADER:
            if(length >= coex_packet_bytes_need) {
                memcpy(&coex_resvered_buffer[coex_resvered_length], p_data, coex_packet_bytes_need);
                coex_resvered_length += coex_packet_bytes_need;
                length -= coex_packet_bytes_need;
                p_data += coex_packet_bytes_need;
            }
            else {
                memcpy(&coex_resvered_buffer[coex_resvered_length], p_data, length);
                coex_resvered_length += length;
                coex_packet_bytes_need -= length;
                length = 0;
                return total_length;
            }
            coex_packet_recv_state = RTKBT_PACKET_CONTENT;

            if(coex_current_type == DATA_TYPE_ACL) {
                coex_packet_bytes_need = *(uint16_t *)&coex_resvered_buffer[2];
            }
             else if(coex_current_type == DATA_TYPE_EVENT){
                coex_packet_bytes_need = coex_resvered_buffer[1];
            }
            else {
                coex_packet_bytes_need = coex_resvered_buffer[2];
            }
            //fall through

        case RTKBT_PACKET_CONTENT:
            if(length >= coex_packet_bytes_need) {
                memcpy(&coex_resvered_buffer[coex_resvered_length], p_data, coex_packet_bytes_need);
                length -= coex_packet_bytes_need;
                p_data += coex_packet_bytes_need;
                coex_resvered_length += coex_packet_bytes_need;
                coex_packet_bytes_need = 0;
            }
            else {
                memcpy(&coex_resvered_buffer[coex_resvered_length], p_data, length);
                coex_resvered_length += length;
                coex_packet_bytes_need -= length;
                length = 0;
                return total_length;
            }
            coex_packet_recv_state = RTKBT_PACKET_END;
            //fall through

        case RTKBT_PACKET_END:
            len = BT_HC_HDR_SIZE + coex_resvered_length;
            p_buf = (HC_BT_HDR *) malloc(len);
            p_buf->offset = 0;
            p_buf->layer_specific = 0;
            p_buf->len = coex_resvered_length;
            memcpy((uint8_t *)(p_buf + 1), coex_resvered_buffer, coex_resvered_length);
            switch (coex_current_type) {
                case DATA_TYPE_EVENT:
                    p_buf->event = MSG_HC_TO_STACK_HCI_EVT;
                    if(rtk_parse_manager)
                        rtk_parse_manager->rtk_parse_internal_event_intercept(coex_resvered_buffer);
                break;

                case DATA_TYPE_ACL:
                    p_buf->event = MSG_HC_TO_STACK_HCI_ACL;
                    handle =  *(uint16_t *)coex_resvered_buffer;
                    acl_length = *(uint16_t *)&coex_resvered_buffer[2];
                    l2cap_length = *(uint16_t *)&coex_resvered_buffer[4];
                    boundary_flag = RTK_GET_BOUNDARY_FLAG(handle);
                    if (boundary_flag == RTK_START_PACKET_BOUNDARY) {
                        if(rtk_parse_manager)
                            rtk_parse_manager->rtk_parse_l2cap_data(coex_resvered_buffer, 0);
                    }
                break;

                case DATA_TYPE_SCO:
                    p_buf->event = MSG_HC_TO_STACK_HCI_SCO;
                break;

                default:
                    p_buf->event = MSG_HC_TO_STACK_HCI_ERR;
                break;
            }
            rtk_btsnoop_capture(p_buf, true);
            free(p_buf);
        break;

        default:

        break;
    }

    coex_packet_recv_state = RTKBT_PACKET_IDLE;
    coex_packet_bytes_need = 0;
    coex_current_type = 0;
    coex_resvered_length = 0;

    return (total_length - length);
}

static void userial_coex_send_data_handler(unsigned char * send_buffer, int total_length)
{
    serial_data_type_t type = 0;
    type = send_buffer[0];
    int length = total_length;
    HC_BT_HDR * p_buf;
    uint8_t boundary_flag;
    uint16_t len, handle, acl_length, l2cap_length;

    len = BT_HC_HDR_SIZE + (length - 1);
    p_buf = (HC_BT_HDR *) malloc(len);
    p_buf->offset = 0;
    p_buf->layer_specific = 0;
    p_buf->len = total_length -1;
    memcpy((uint8_t *)(p_buf + 1), &send_buffer[1], length - 1);

    switch (type) {
        case DATA_TYPE_COMMAND:
            p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
            if(rtk_parse_manager)
                rtk_parse_manager->rtk_parse_command(&send_buffer[1]);
        break;

        case DATA_TYPE_ACL:
            p_buf->event = MSG_STACK_TO_HC_HCI_ACL;
            handle =  *(uint16_t *)&send_buffer[1];
            acl_length = *(uint16_t *)&send_buffer[3];
            l2cap_length = *(uint16_t *)&send_buffer[5];
            boundary_flag = RTK_GET_BOUNDARY_FLAG(handle);
            if (boundary_flag == RTK_START_PACKET_BOUNDARY) {
                if(rtk_parse_manager)
                    rtk_parse_manager->rtk_parse_l2cap_data(&send_buffer[1], 1);
            }

        break;

        case DATA_TYPE_SCO:
            p_buf->event = MSG_STACK_TO_HC_HCI_SCO;
        break;
        default:
            p_buf->event = 0;
            ALOGE("%s invalid data type: %d", __func__, type);
        break;
    }
    rtk_btsnoop_capture(p_buf, false);
    free(p_buf);
}

static void userial_coex_handler(void *context)
{
    RTK_BUFFER* skb_data;
    RTK_BUFFER* skb_type;
    eventfd_t value;
    int read_length = 0;
    eventfd_read(vnd_userial.event_fd, &value);
    if(!value && !vnd_userial.thread_running) {
        return;
    }

    while(!RtbQueueIsEmpty(vnd_userial.data_order)) {
        read_length = 0;
        skb_type = RtbDequeueHead(vnd_userial.data_order);
        if(skb_type) {
            if(*(skb_type->Data) == RTK_DATA_RECEIVED) {
                skb_data = RtbDequeueHead(vnd_userial.recv_data);
                if(skb_data) {
                    do {
                        read_length += userial_coex_recv_data_handler((skb_data->Data + read_length), (skb_data->Length - read_length));
                    }while(read_length < skb_data->Length);
                    RtbFree(skb_data);
                }
            }
            else {
                skb_data = RtbDequeueHead(vnd_userial.send_data);
                if(skb_data) {
                    userial_coex_send_data_handler(skb_data->Data, skb_data->Length);
                    RtbFree(skb_data);
                }

            }

            RtbFree(skb_type);
        }
    }
}

static void userial_recv_H4_rawdata(void *context)
{
    serial_data_type_t type = 0;
    ssize_t bytes_read;
    uint16_t opcode;
    uint16_t transmitted_length = 0;
    unsigned char *buffer = NULL;

    switch (packet_recv_state) {
        case RTKBT_PACKET_IDLE:
            packet_bytes_need = 1;
            do {
                RTK_NO_INTR(bytes_read = read(vnd_userial.uart_fd[1], &type, 1));
                if(bytes_read == -1) {
                    ALOGE("%s, state = %d, read error %s", __func__, packet_recv_state, strerror(errno));
                    return;
                }
                if(!bytes_read && packet_bytes_need) {
                    ALOGE("%s, state = %d, bytes_read 0", __func__, packet_recv_state);
                    return;
                }
                assert((type >= DATA_TYPE_COMMAND) && (type <= DATA_TYPE_SCO));
                if (type < DATA_TYPE_COMMAND || type > DATA_TYPE_SCO) {
                    ALOGE("%s invalid data type: %d", __func__, type);
                }
                else {
                    packet_bytes_need -= bytes_read;
                    packet_recv_state = RTKBT_PACKET_TYPE;
                    current_type = type;
                    h4_read_buffer[0] = type;
                }
            }while(packet_bytes_need);
            //fall through

        case RTKBT_PACKET_TYPE:
            if(current_type == DATA_TYPE_ACL) {
                packet_bytes_need = 4;
            }
            else {
                packet_bytes_need = 3;
            }
            h4_read_length = 0;
            packet_recv_state = RTKBT_PACKET_HEADER;
            //fall through

        case RTKBT_PACKET_HEADER:
            do {
                RTK_NO_INTR(bytes_read = read(vnd_userial.uart_fd[1], &h4_read_buffer[h4_read_length + 1], packet_bytes_need));
                if(bytes_read == -1) {
                    ALOGE("%s, state = %d, read error %s", __func__, packet_recv_state, strerror(errno));
                    return;
                }
                if(!bytes_read && packet_bytes_need) {
                    ALOGE("%s, state = %d, bytes_read 0", __func__, packet_recv_state);
                    return;
                }
                packet_bytes_need -= bytes_read;
                h4_read_length += bytes_read;
            }while(packet_bytes_need);
            packet_recv_state = RTKBT_PACKET_CONTENT;

            if(current_type == DATA_TYPE_ACL) {
                packet_bytes_need = *(uint16_t *)&h4_read_buffer[3];
            } else {
                packet_bytes_need = h4_read_buffer[3];
            }
            //fall through

        case RTKBT_PACKET_CONTENT:
            while(packet_bytes_need) {
                RTK_NO_INTR(bytes_read = read(vnd_userial.uart_fd[1], &h4_read_buffer[h4_read_length + 1], packet_bytes_need));
                if(bytes_read == -1) {
                    ALOGE("%s, state = %d, read error %s", __func__, packet_recv_state, strerror(errno));
                    return;
                }
                if(!bytes_read) {
                    ALOGE("%s, state = %d, bytes_read 0", __func__, packet_recv_state);
                    return;
                }

                packet_bytes_need -= bytes_read;
                h4_read_length += bytes_read;
            }
            packet_recv_state = RTKBT_PACKET_END;
            //fall through

        case RTKBT_PACKET_END:
            switch (current_type) {
                case DATA_TYPE_COMMAND:
                    if(rtkbt_transtype & RTKBT_TRANS_H4) {
                        h4_int_transmit_data(h4_read_buffer, (h4_read_length + 1));
                    }
                        else {
                        opcode = *(uint16_t *)&h4_read_buffer[1];
                        if(opcode == HCI_VSC_H5_INIT) {
                            h5_int_interface->h5_send_sync_cmd(opcode, NULL, h4_read_length);
                        }
                        else {
                            transmitted_length = h5_int_interface->h5_send_cmd(type, &h4_read_buffer[1], h4_read_length);
                        }
                    }
                break;

                case DATA_TYPE_ACL:
                    if(rtkbt_transtype & RTKBT_TRANS_H4) {
                        h4_int_transmit_data(h4_read_buffer, (h4_read_length + 1));
                    }
                    else {
                        transmitted_length = h5_int_interface->h5_send_acl_data(type, &h4_read_buffer[1], h4_read_length);
                    }
                break;

                case DATA_TYPE_SCO:
                    if(rtkbt_transtype & RTKBT_TRANS_H4) {
                        h4_int_transmit_data(h4_read_buffer, (h4_read_length + 1));
                    }
                    else {
                        transmitted_length = h5_int_interface->h5_send_sco_data(type, &h4_read_buffer[1], h4_read_length);
                    }
                break;
                default:
                    ALOGE("%s invalid data type: %d", __func__, current_type);
                break;
            }

            userial_enqueue_coex_rawdata(h4_read_buffer,(h4_read_length + 1), false);
        break;

        default:

        break;
    }

    packet_recv_state = RTKBT_PACKET_IDLE;
    packet_bytes_need = 0;
    current_type = 0;
    h4_read_length = 0;
}

static uint16_t h5_int_transmit_data_cb(serial_data_type_t type, uint8_t *data, uint16_t length) {
    assert(data != NULL);
    assert(length > 0);

    if (type != DATA_TYPE_H5) {
        ALOGE("%s invalid data type: %d", __func__, type);
        return 0;
    }

    uint16_t transmitted_length = 0;
    while (length > 0) {
        ssize_t ret = write(vnd_userial.fd, data + transmitted_length, length);
        switch (ret) {
            case -1:
            ALOGE("In %s, error writing to the uart serial port: %s", __func__, strerror(errno));
            goto done;
        case 0:
            // If we wrote nothing, don't loop more because we
            // can't go to infinity or beyond, ohterwise H5 can resend data
            ALOGE("%s, ret %d", __func__, ret);
            goto done;
        default:
            transmitted_length += ret;
            length -= ret;
            break;
        }
    }

done:;
    return transmitted_length;

}

static void h5_data_ready_cb(serial_data_type_t type, unsigned int total_length)
{
    unsigned char buffer[1028] = {0};
    unsigned length = 0;
    length = h5_int_interface->h5_int_read_data(&buffer[1], total_length);
    buffer[0] = type;
    length++;
    uint16_t transmitted_length = 0;
    while (length > 0) {
        ssize_t ret;
        RTK_NO_INTR(ret = write(vnd_userial.uart_fd[1], buffer + transmitted_length, length));
        switch (ret) {
        case -1:
            ALOGE("In %s, error writing to the uart serial port: %s", __func__, strerror(errno));
            goto done;
        case 0:
            // If we wrote nothing, don't loop more because we
            // can't go to infinity or beyond
            goto done;
        default:
            transmitted_length += ret;
            length -= ret;
            break;
        }
    }
done:;
    return;
}


#ifdef RTK_SINGLE_CMD_EVENT
static int userial_handle_cmd_event(unsigned char * recv_buffer, int total_length)
{
    serial_data_type_t type = 0;
    unsigned char * p_data = recv_buffer;
    int length = total_length;
    uint8_t event;
    switch (cmd_packet_recv_state) {
        case RTKBT_PACKET_IDLE:
            cmd_packet_bytes_need = 1;
            while(length) {
                type = p_data[0];
                length--;
                p_data++;
                assert((type > DATA_TYPE_COMMAND) && (type <= DATA_TYPE_EVENT));
                if (type < DATA_TYPE_ACL || type > DATA_TYPE_EVENT) {
                    ALOGE("%s invalid data type: %d", __func__, type);
                    if(!length)
                        return total_length;

                    continue;
                }
                break;
            }
            cmd_current_type = type;
            cmd_packet_recv_state = RTKBT_PACKET_TYPE;
            //fall through

        case RTKBT_PACKET_TYPE:
            if(cmd_current_type == DATA_TYPE_ACL) {
                cmd_packet_bytes_need = 4;
            }
            else if(cmd_current_type == DATA_TYPE_EVENT) {
                cmd_packet_bytes_need = 2;
            }
            else {
                cmd_packet_bytes_need = 3;
            }
            cmd_resvered_length = 0;
            cmd_packet_recv_state = RTKBT_PACKET_HEADER;
            //fall through

        case RTKBT_PACKET_HEADER:
            if(length >= cmd_packet_bytes_need) {
                memcpy(&cmd_resvered_header[cmd_resvered_length], p_data, cmd_packet_bytes_need);
                cmd_resvered_length += cmd_packet_bytes_need;
                length -= cmd_packet_bytes_need;
                p_data += cmd_packet_bytes_need;
            }
            else {
                memcpy(&cmd_resvered_header[cmd_resvered_length], p_data, length);
                cmd_resvered_length += length;
                cmd_packet_bytes_need -= length;
                length = 0;
                return total_length;
            }
            cmd_packet_recv_state = RTKBT_PACKET_CONTENT;

            if(cmd_current_type == DATA_TYPE_ACL) {
                cmd_packet_bytes_need = *(uint16_t *)&cmd_resvered_header[2];
            }
             else if(cmd_current_type == DATA_TYPE_EVENT){
                cmd_packet_bytes_need = cmd_resvered_header[1];
            }
            else {
                cmd_packet_bytes_need = cmd_resvered_header[2];
            }
            //fall through

        case RTKBT_PACKET_CONTENT:
            if(cmd_current_type == DATA_TYPE_EVENT) {
                event = cmd_resvered_header[0];
                if(cmd_resvered_length == 2) {
                    if(event == HCI_COMMAND_COMPLETE_EVT) {
                        if(length > 1) {
                            *p_data = 1;
                        }
                    }
                    else if(event == HCI_COMMAND_STATUS_EVT) {
                        if(length > 2) {
                            *(p_data + 1) = 1;
                        }
                    }
                }
                else if(cmd_resvered_length == 3) {
                    if(event == HCI_COMMAND_STATUS_EVT) {
                        if(length > 1) {
                            *p_data = 1;
                        }
                    }

                }
            }
            if(length >= cmd_packet_bytes_need) {
                length -= cmd_packet_bytes_need;
                p_data += cmd_packet_bytes_need;
                cmd_resvered_length += cmd_packet_bytes_need;
                cmd_packet_bytes_need = 0;
            }
            else {
                cmd_resvered_length += length;
                cmd_packet_bytes_need -= length;
                length = 0;
                return total_length;
            }
            cmd_packet_recv_state = RTKBT_PACKET_END;
            //fall through

        case RTKBT_PACKET_END:

        break;

        default:

        break;
    }

    cmd_packet_recv_state = RTKBT_PACKET_IDLE;
    cmd_packet_bytes_need = 0;
    cmd_current_type = 0;
    cmd_resvered_length = 0;

    return (total_length - length);
}
#endif

static void userial_recv_uart_rawdata(unsigned char *buffer, unsigned int total_length)
{
    unsigned int length = total_length;
    uint16_t transmitted_length = 0;
#ifdef RTK_SINGLE_CMD_EVENT
    int read_length = 0;
    do {
        read_length += userial_handle_cmd_event(buffer + read_length, total_length - read_length);

    }while(read_length < total_length);
#endif
    while (length > 0) {
        ssize_t ret;
        RTK_NO_INTR(ret = write(vnd_userial.uart_fd[1], buffer + transmitted_length, length));
        switch (ret) {
        case -1:
            ALOGE("In %s, error writing to the uart serial port: %s", __func__, strerror(errno));
            goto done;
        case 0:
            // If we wrote nothing, don't loop more because we
            // can't go to infinity or beyond
            goto done;
        default:
            transmitted_length += ret;
            length -= ret;
            break;
        }
    }
done:;
    if(total_length)
        userial_enqueue_coex_rawdata(buffer, total_length, true);
    return;
}

static void* userial_recv_socket_thread(void *arg)
{
    struct epoll_event events[64];
    int j;
    while(vnd_userial.thread_running) {
        int ret;
        do{
            ret = epoll_wait(vnd_userial.epoll_fd, events, 32, 500);
        }while(vnd_userial.thread_running && ret == -1 && errno == EINTR);
        if (ret == -1) {
            ALOGE("%s error in epoll_wait: %s", __func__, strerror(errno));
        }
        for (j = 0; j < ret; ++j) {
            struct rtk_object_t *object = (struct rtk_object_t *)events[j].data.ptr;
            if (events[j].data.ptr == NULL)
                continue;
            else {
                if (events[j].events & (EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR) && object->read_ready)
                    object->read_ready(object->context);
                if (events[j].events & EPOLLOUT && object->write_ready)
                    object->write_ready(object->context);
            }
        }
    }
    ALOGD("%s exit", __func__);
    return NULL;
}

static void* userial_recv_uart_thread(void *arg)
{
    struct pollfd pfd;
    pfd.events = POLLIN|POLLHUP|POLLERR|POLLRDHUP;
    pfd.fd = vnd_userial.fd;
    int ret;
    unsigned char read_buffer[2056] = {0};
    ssize_t bytes_read;
    while(vnd_userial.thread_running) {
        do{
            ret = poll(&pfd, 1, 50);
        }while(ret == -1 && errno == EINTR && vnd_userial.thread_running);

        if (pfd.revents & POLLIN) {
            RTK_NO_INTR(bytes_read = read(vnd_userial.fd, read_buffer, sizeof(read_buffer)));
            if(!bytes_read)
                continue;

            if(rtkbt_transtype & RTKBT_TRANS_H5) {
                h5_int_interface->h5_recv_msg(read_buffer, bytes_read);
            }
            else {
                userial_recv_uart_rawdata(read_buffer, bytes_read);
            }
        }

        if (pfd.revents & (POLLERR|POLLHUP)) {
            ALOGE("%s poll error, fd : %d", __func__, vnd_userial.fd);
            continue;
        }
        if (ret < 0)
        {
            ALOGE("%s : error (%d)", __func__, ret);
            continue;
        }
    }
    ALOGD("%s exit", __func__);
    return NULL;
}

static void* userial_coex_thread(void *arg)
{
    struct epoll_event events[64];
    int j;
    while(vnd_userial.thread_running) {
        int ret;
        do{
            ret = epoll_wait(vnd_userial.cpoll_fd, events, 64, 500);
        }while(ret == -1 && errno == EINTR && vnd_userial.thread_running);
        if (ret == -1) {
            ALOGE("%s error in epoll_wait: %s", __func__, strerror(errno));
        }
        for (j = 0; j < ret; ++j) {
            struct rtk_object_t *object = (struct rtk_object_t *)events[j].data.ptr;
            if (events[j].data.ptr == NULL)
                continue;
            else {
                if (events[j].events & (EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR) && object->read_ready)
                    object->read_ready(object->context);
                if (events[j].events & EPOLLOUT && object->write_ready)
                    object->write_ready(object->context);
            }
        }
    }
    ALOGD("%s exit", __func__);
    return NULL;
}

int userial_socket_open()
{
    int ret = 0;
    struct epoll_event event;
    if((ret = socketpair(AF_UNIX, SOCK_STREAM, 0, vnd_userial.uart_fd)) < 0) {
        ALOGE("%s, errno : %s", __func__, strerror(errno));
        return ret;
    }

    vnd_userial.epoll_fd = epoll_create(64);
    if (vnd_userial.epoll_fd == -1) {
        ALOGE("%s unable to create epoll instance: %s", __func__, strerror(errno));
        return -1;
    }

    rtk_socket_object.fd = vnd_userial.uart_fd[1];
    rtk_socket_object.read_ready = userial_recv_H4_rawdata;
    memset(&event, 0, sizeof(event));
    event.events |= EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR;
    event.data.ptr = (void *)&rtk_socket_object;
    if (epoll_ctl(vnd_userial.epoll_fd, EPOLL_CTL_ADD, vnd_userial.uart_fd[1], &event) == -1) {
        ALOGE("%s unable to register fd %d to epoll set: %s", __func__, vnd_userial.uart_fd[1], strerror(errno));
        close(vnd_userial.epoll_fd);
        return -1;
    }

    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
    vnd_userial.thread_running = true;
    if (pthread_create(&vnd_userial.thread_socket_id, &thread_attr, userial_recv_socket_thread, NULL)!=0 )
    {
        ALOGE("pthread_create : %s", strerror(errno));
        close(vnd_userial.epoll_fd);
        return -1;
    }


    if (pthread_create(&vnd_userial.thread_uart_id, &thread_attr, userial_recv_uart_thread, NULL)!=0 )
    {
        ALOGE("pthread_create : %s", strerror(errno));
        close(vnd_userial.epoll_fd);
        vnd_userial.thread_running = false;
        pthread_join(vnd_userial.thread_socket_id, NULL);
        return -1;
    }

    vnd_userial.cpoll_fd = epoll_create(64);
    assert (vnd_userial.cpoll_fd != -1);

    vnd_userial.event_fd = eventfd(SIZE_MAX, EFD_NONBLOCK);
    assert(vnd_userial.event_fd != -1);
    if(vnd_userial.event_fd != -1) {
        rtk_coex_object.fd = vnd_userial.event_fd;
        rtk_coex_object.read_ready = userial_coex_handler;
        memset(&event, 0, sizeof(event));
        event.events |= EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR;
        event.data.ptr = (void *)&rtk_coex_object;
        if (epoll_ctl(vnd_userial.cpoll_fd, EPOLL_CTL_ADD, vnd_userial.event_fd, &event) == -1) {
            ALOGE("%s unable to register fd %d to cpoll set: %s", __func__, vnd_userial.event_fd, strerror(errno));
            assert(false);
        }

        if (pthread_create(&vnd_userial.thread_coex_id, &thread_attr, userial_coex_thread, NULL) !=0 )
        {
            ALOGE("pthread create  coex : %s", strerror(errno));
            assert(false);
        }
    }
    ret = vnd_userial.uart_fd[0];
    return ret;
}

int userial_vendor_usb_ioctl(int operation)
{
    int retval;
    retval = ioctl(vnd_userial.fd, operation, NULL);
    return retval;
}

int userial_vendor_usb_open(void)
{
	if ((vnd_userial.fd = open(vnd_userial.port_name, O_RDWR)) == -1)
	{
	    ALOGE("%s: unable to open %s: %s", __func__, vnd_userial.port_name, strerror(errno));
	    return -1;
	}

	ALOGI("device fd = %d open", vnd_userial.fd);

	return vnd_userial.fd;
}

