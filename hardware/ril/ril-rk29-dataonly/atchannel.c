/* //device/system/reference-ril/atchannel.c
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
#include <telephony/ril.h>
#include "atchannel.h"
#include "at_tok.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define LOG_NDEBUG 0
#define LOG_TAG "AT"
#include <utils/Log.h>

#include "misc.h"


#define NUM_ELEMS(x) (sizeof(x)/sizeof(x[0]))

#define MAX_AT_RESPONSE (8 * 1024)
#define HANDSHAKE_RETRY_COUNT 8
#define HANDSHAKE_TIMEOUT_MSEC 250

// cmy: 在发送AT命令之前加点延时，防止多个命令下发时底层处理不过来
//#define DELAY_BEFORE_AT_CMD_MS     100
static pthread_t s_tid_reader;
static int s_fd = -1;    /* fd of the AT channel */
static ATUnsolHandler s_unsolHandler;

/* for input buffering */

static char s_ATBuffer[MAX_AT_RESPONSE+1];
static char *s_ATBufferCur = s_ATBuffer;

static int s_ackPowerIoctl; /* true if TTY has android byte-count
                                handshake for low power*/
static int s_readCount = 0;

#if AT_DEBUG
void  AT_DUMP(const char*  prefix, const char*  buff, int  len)
{
    if (len < 0)
        len = strlen(buff);
    LOGD("%.*s", len, buff);
}
#endif

/*
 * for current pending command
 * these are protected by s_commandmutex
 */

static pthread_mutex_t s_commandmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_commandcond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_writeMutex = PTHREAD_MUTEX_INITIALIZER;

static ATCommandType s_type;
static const char *s_responsePrefix = NULL;
static const char *s_smsPDU = NULL;
static ATResponse *sp_response = NULL;

static void (*s_onTimeout)(void) = NULL;
static void (*s_onReaderClosed)(void) = NULL;
static int s_readerClosed;

static void onReaderClosed();
static int writeCtrlZ (const char *s);
static int writeline (const char *s);
#define NS_PER_S 1000000000

#define LG_VL600

#ifdef LG_VL600
static unsigned int send_seq = 0;
static unsigned int recv_seq = -1;
static const unsigned char VL600_CMD_HEADER[] = 
{
  0x5A, 0x48, 0x12, 0xA5, 
  0, 0, 0, 0,
  0, 0, 0, 0,
  0x11, 0xF0
};
static const size_t VL600_CMD_HEADER_LEN = sizeof(VL600_CMD_HEADER)*sizeof(unsigned char);

static unsigned char vl600_attach_cmd[1908] = 
{
  0x5A, 0x48, 0x12, 0xA5, 
  0, 0, 0, 0,
  0, 0, 0, 0,
  0x21, 0xF0,
};

static char in_buf[MAX_AT_RESPONSE];
static char out_buf[MAX_AT_RESPONSE];

static void dump_pack(unsigned char* data, int len)
{
    int i=0;
    int j=0;
    char bs[3];
    char out[128] = "\0";

    int line = (len+15)/16;
    for(i=0; i<line; i++)
    {
        for(j=0; j<16; j++)
        {
            if((i*16+j)>=len)
                break;
            sprintf(bs, " %02X", data[i*16+j]);
            strcat(out, bs);
        }
        LOGD("%s", out);
        memset(out, 0, 128);
//        strcat(out, "\n");
    }
}

/*
    return value:
        success  > 0
        failed   <= 0
 */
static int vl600_pack(const char* in, unsigned char* out, int buf_len)
{
    int len = strlen(in);
    int out_len = VL600_CMD_HEADER_LEN+len;

//    LOGD("vl600 packet");
//    dump_pack(in, len);
    
    out_len += 4-(out_len%4);
//    LOGD("in_len=%d, out_len=%d", len, out_len);

    if( out_len > buf_len )
    {
        LOGD("Out buffer not enough");
        return 0;
    }
    
    memset(out, 0, out_len);
    memcpy(out, VL600_CMD_HEADER, VL600_CMD_HEADER_LEN);
    *(unsigned int*)(out+4) = send_seq;
    *(unsigned int*)(out+8) = len;
    memcpy(out+VL600_CMD_HEADER_LEN, in, len);

//    dump_pack(out, out_len);

    send_seq++;
//    LOGD("send_seq=%d", send_seq);
    
    return out_len;
}

/*
    return value:
        success  >= 0
        failed   < 0
 */
static int vl600_unpack(const unsigned char *in, int in_len, const unsigned char *out, int max_len)
{
    const unsigned char* p = in;
    int p_len = in_len;
    unsigned int seq = 0;
    int len = 0;
    int out_len;

//    LOGD("vl600 unpacket");
//    dump_pack(in, in_len);
    
    while(*p == 0){
        ++p;
        --p_len;
    }

    if(p_len<=14)
    {
        LOGE("data len=%d", p_len);
        return -1;
    }

    if( p[0] != 0x5a || p[1] != 0x48 || p[2] != 0x12 || p[3] != 0xa5 ||
        p[12] != 0x11 || p[13] != 0xf0)
    {
        LOGE("Invalid magic");
        return -1;
    }

    seq = *(unsigned int*)(p+4);
//    LOGD("seq=%d", seq);

    if(seq == recv_seq)
    {
        LOGE("Invalid sequence");
        return -1;
    }

    recv_seq = seq;
    
    len = *(unsigned int*)(p+8);
//    LOGD("len=%d, p_len=%d", len, p_len);

    if(p_len < len+VL600_CMD_HEADER_LEN)
    {
        LOGE("Packet incomplete");
        return -1;
    }
    out_len = len>max_len?max_len:len;
    memcpy(out, p+VL600_CMD_HEADER_LEN, out_len);
    
    if(len > max_len)
    {
        LOGD("packet too long");
    }

//    dump_pack(out, out_len);

    return out_len;
}

static unsigned char* vl600_setup_attach()
{
    unsigned char* cmd = vl600_attach_cmd;
    unsigned int plen = 1891;

    *(unsigned int*)(cmd+4) = send_seq;
    *(unsigned int*)(cmd+8) = plen;
    cmd[14] = 0xF1;
    cmd[15] = 0x4A;
    cmd[1908-6]=0xB1;
    cmd[1908-5]=0xF3;
    cmd[1908-4]=0x7E;
    
//    dump_pack(cmd, 1908);
    
    return cmd;
}

int vl600_attach()
{
    size_t cur = 0;
    ssize_t written;

    LOGD("vl600_attach");

    if (s_fd < 0 || s_readerClosed > 0) {
        return AT_ERROR_CHANNEL_CLOSED;
    }

    unsigned char *cmd = vl600_setup_attach();
    size_t len = sizeof(vl600_attach_cmd);

    /* the main string */
    while (cur < len) {
        do {
            written = write (s_fd, cmd + cur, len - cur);
        } while (written < 0 && errno == EINTR);

        if (written < 0) {
            return AT_ERROR_GENERIC;
        }

        cur += written;
    }

    return 0;
}
#endif

static void sleepMsec(long long msec);
static int is_emulate = 0;
static char cur_cmd[1024] = {0};

int at_emulate_enter()
{
    is_emulate = 1;
    sleepMsec(200);
    
    close(s_fd);
    return 0;
}

extern int open_at_port ();
int at_emulate_exit()
{
    if(is_emulate)
    {
        s_fd = open_at_port();
        sleepMsec(1000);
        
        is_emulate = 0;

        if(s_fd > 0 )
            at_handshake();
    }
    return 0;
}

#if 1// UML290
    char reponse_csq[256] = "+CSQ: 21, 99\r";
    char reponse_cimi[256] = "311480009868905\r";
    char reponse_cops[256] = "+COPS: 0,0,\"Verizon Wireless\",7\r";
    char reponse_cgmm[256] = "+CGMM: UML290VW\r";
    char reponse_cgsn[256] = "+CGSN: 990000475112084\r";
    char reponse_creg[256] = "+CREG: 2,1\r";
    char reponse_cgreg[256] = "+CGREG: 2,1, FFFE, 4EEC03, 7\r";
    char reponse_sysinfo[256] = "^SYSINFO: 2,3,0,15,1,0,5\r";
    char reponse_cgact[256] = "+CGACT: 1,1\r+CGACT: 3,1\r+CGACT: 4,0\r";
    char reponse_cgdcont[256] = "+CGDCONT: 3,\"IPV4V6\",\"vzwinternet\",\"0.0.0.0\",0,0\r";
#else
    char reponse_csq[256] = "+CSQ: 10, 99\r";
    char reponse_cimi[256] = "111110001111905\r";
    char reponse_cops[256] = "+COPS: 0,0,\"TEST\",2\r";
    char reponse_cgmm[256] = "+CGMM: TEST\r";
    char reponse_cgsn[256] = "+CGSN: 110000115112084\r";
    char reponse_creg[256] = "+CREG: 2,1\r";
    char reponse_cgreg[256] = "+CGREG: 2,1, FFFE, 4EEC03\r";
    char reponse_sysinfo[256] = "^SYSINFO: 2,3,0,15,1,0,5\r";
    char reponse_cgact[256] = "+CGACT: 1,1\r";
    char reponse_cgdcont[256] = "+CGDCONT: 1,\"IP\",\"cmnet\",,,0,0\r";
#endif

static int read_emulate(char* buf, int len)
{
    int ret = 0;

    while(cur_cmd[0] == 0)
    {
        if(!is_emulate)
        {
            if(s_fd<0)
                return -1;
            else
                break;
        }
        sleepMsec(20);
    }

    if ( strcmp(cur_cmd, "AT+CSQ")==0 )
    {
        strcpy(buf, reponse_csq);
        ret = strlen(reponse_csq);
    }
    else if ( strcmp(cur_cmd, "AT+CFUN?")==0 )
    {
        strcpy(buf, "+CFUN: 1\r");
        ret = strlen("+CFUN: 1\r");
    }
    else if ( strcmp(cur_cmd, "AT+CPIN?")==0 )
    {
        strcpy(buf, "+CPIN: READY\r");
        ret = strlen("+CPIN: READY\r");
    }
    else if ( strcmp(cur_cmd, "AT+CIMI")==0 )
    {
        strcpy(buf, reponse_cimi);
        ret = strlen(reponse_cimi);
    }
    else if ( strcmp(cur_cmd, "AT+COPS?")==0 )
    {
        strcpy(buf, reponse_cops);
        ret = strlen(reponse_cops);
    }
    else if ( strcmp(cur_cmd, "AT+CGMM")==0 )
    {
        strcpy(buf, reponse_cgmm);
        ret = strlen(reponse_cgmm);
    }
    else if ( strcmp(cur_cmd, "AT^MEID")==0 )
    {
        strcpy(buf, "^MEID: 12345678901234\r");
        ret = strlen("^MEID: 12345678901234\r");
    }
    else if ( strcmp(cur_cmd, "AT+GSN")==0 )
    {
        strcpy(buf, "+GSN: 86e5515f\r");
        ret = strlen("+GSN: 86e5515f\r");
    }
    else if ( strcmp(cur_cmd, "AT+CGSN")==0 )
    {
        strcpy(buf, reponse_cgsn);
        ret = strlen(reponse_cgsn);
    }
    else if ( strcmp(cur_cmd, "AT+CREG?")==0 )
    {
        strcpy(buf, reponse_creg);
        ret = strlen(reponse_creg);
    }
    else if ( strcmp(cur_cmd, "AT+CGREG?")==0 )
    {
        strcpy(buf, reponse_cgreg);
        ret = strlen(reponse_cgreg);
    }
    else if ( strcmp(cur_cmd, "AT^SYSINFO")==0 )
    {
        strcpy(buf, reponse_sysinfo);
        ret = strlen(reponse_sysinfo);
    }
    else if ( strcmp(cur_cmd, "AT+CGACT?")==0 )
    {
        strcpy(buf, reponse_cgact);
        ret = strlen(reponse_cgact);
    }
    else if ( strcmp(cur_cmd, "AT+CGDCONT?")==0 )
    {
        strcpy(buf, reponse_cgdcont);
        ret = strlen(reponse_cgdcont);
    }
    
    strcat(buf, "OK\r");
    ret += strlen("OK\r");

    return ret;
}

static void setTimespecRelative(struct timespec *p_ts, long long msec)
{
    struct timeval tv;

    gettimeofday(&tv, (struct timezone *) NULL);

    p_ts->tv_sec = tv.tv_sec + (msec / 1000);
    p_ts->tv_nsec = (tv.tv_usec + (msec % 1000) * 1000L ) * 1000L;
    /* assuming tv.tv_usec < 10^6 */
    if (p_ts->tv_nsec >= NS_PER_S) {
        p_ts->tv_sec++;
        p_ts->tv_nsec -= NS_PER_S;
    }
}

static void sleepMsec(long long msec)
{
    struct timespec ts;
    int err;

    ts.tv_sec = (msec / 1000);
    ts.tv_nsec = (msec % 1000) * 1000 * 1000;

    do {
        err = nanosleep (&ts, &ts);
    } while (err < 0 && errno == EINTR);
}



/** add an intermediate response to sp_response*/
static void addIntermediate(const char *line)
{
    ATLine *p_new;

    p_new = (ATLine  *) malloc(sizeof(ATLine));

    p_new->line = strdup(line);

    /* note: this adds to the head of the list, so the list
       will be in reverse order of lines received. the order is flipped
       again before passing on to the command issuer */
    p_new->p_next = sp_response->p_intermediates;
    sp_response->p_intermediates = p_new;
}


/**
 * returns 1 if line is a final response indicating error
 * See 27.007 annex B
 * WARNING: NO CARRIER and others are sometimes unsolicited
 */
static const char * s_finalResponsesError[] = {
    "ERROR",
    "+CMS ERROR:",
    "+CME ERROR:",
    "NO CARRIER", /* sometimes! */
    "NO ANSWER",
    "NO DIALTONE",
    
// cmy: 下面几个出错是HuaWei EM660C模块命令出错信息    
    "COMMAND NOT SUPPORT",
    "TOO MANY PARAMETERS",
};
static int isFinalResponseError(const char *line)
{
    size_t i;

    for (i = 0 ; i < NUM_ELEMS(s_finalResponsesError) ; i++) {
        if (strStartsWith(line, s_finalResponsesError[i])) {
            return 1;
        }
    }

    return 0;
}

/**
 * returns 1 if line is a final response indicating success
 * See 27.007 annex B
 * WARNING: NO CARRIER and others are sometimes unsolicited
 */
static const char * s_finalResponsesSuccess[] = {
    "OK",
    "CONNECT"       /* some stacks start up data on another channel */
};
static int isFinalResponseSuccess(const char *line)
{
    size_t i;

    for (i = 0 ; i < NUM_ELEMS(s_finalResponsesSuccess) ; i++) {
        if (strStartsWith(line, s_finalResponsesSuccess[i])) {
            return 1;
        }
    }

    return 0;
}

/**
 * returns 1 if line is a final response, either  error or success
 * See 27.007 annex B
 * WARNING: NO CARRIER and others are sometimes unsolicited
 */
static int isFinalResponse(const char *line)
{
    return isFinalResponseSuccess(line) || isFinalResponseError(line);
}


/**
 * returns 1 if line is the first line in (what will be) a two-line
 * SMS unsolicited response
 */
static const char * s_smsUnsoliciteds[] = {
    "+CMT:",
    "+CDS:",
    "+CBM:"
};
static int isSMSUnsolicited(const char *line)
{
    size_t i;

    for (i = 0 ; i < NUM_ELEMS(s_smsUnsoliciteds) ; i++) {
        if (strStartsWith(line, s_smsUnsoliciteds[i])) {
            return 1;
        }
    }

    return 0;
}


/** assumes s_commandmutex is held */
static void handleFinalResponse(const char *line)
{
    sp_response->finalResponse = strdup(line);

    pthread_cond_signal(&s_commandcond);
}

static void handleUnsolicited(const char *line)
{
    if (s_unsolHandler != NULL) {
        s_unsolHandler(line, NULL);
    }
}

static void processLine(const char *line)
{
    pthread_mutex_lock(&s_commandmutex);

    if (sp_response == NULL) {
        /* no command pending */
        handleUnsolicited(line);
    } else if (isFinalResponseSuccess(line)) {
        sp_response->success = 1;
        handleFinalResponse(line);
    } else if (isFinalResponseError(line)) {
        sp_response->success = 0;
        handleFinalResponse(line);
    } else if (s_smsPDU != NULL && 0 == strcmp(line, "> ")) {
        // See eg. TS 27.005 4.3
        // Commands like AT+CMGS have a "> " prompt
        writeCtrlZ(s_smsPDU);
        s_smsPDU = NULL;
    } else switch (s_type) {
        case NO_RESULT:
            handleUnsolicited(line);
            break;
        case NUMERIC:
            if (sp_response->p_intermediates == NULL
                && isdigit(line[0])
            ) {
                addIntermediate(line);
            } else {
                /* either we already have an intermediate response or
                   the line doesn't begin with a digit */
                handleUnsolicited(line);
            }
            break;
        case SINGLELINE:
            if (sp_response->p_intermediates == NULL
                && strStartsWith (line, s_responsePrefix)
            ) {
                addIntermediate(line);
            } else {
                /* we already have an intermediate response */
                handleUnsolicited(line);
            }
            break;
        case MULTILINE:
            if (strStartsWith (line, s_responsePrefix)) {
                addIntermediate(line);
            } else {
                handleUnsolicited(line);
            }
        break;

        default: /* this should never be reached */
            LOGE("Unsupported AT command type %d\n", s_type);
            handleUnsolicited(line);
        break;
    }

    pthread_mutex_unlock(&s_commandmutex);
}


/**
 * Returns a pointer to the end of the next line
 * special-cases the "> " SMS prompt
 *
 * returns NULL if there is no complete line
 */
static char * findNextEOL(char *cur)
{
    if (cur[0] == '>' && cur[1] == ' ' && cur[2] == '\0') {
        /* SMS prompt character...not \r terminated */
        return cur+2;
    }

    // Find next newline
    while (*cur != '\0' && *cur != '\r' && *cur != '\n') cur++;

    return *cur == '\0' ? NULL : cur;
}


/**
 * Reads a line from the AT channel, returns NULL on timeout.
 * Assumes it has exclusive read access to the FD
 *
 * This line is valid only until the next call to readline
 *
 * This function exists because as of writing, android libc does not
 * have buffered stdio.
 */

extern int modem_cmp(int vid, int pid, const char* name);
static const char *readline()
{
    ssize_t count;

    char *p_read = NULL;
    char *p_eol = NULL;
    char *ret;

    /* this is a little odd. I use *s_ATBufferCur == 0 to
     * mean "buffer consumed completely". If it points to a character, than
     * the buffer continues until a \0
     */
    if (*s_ATBufferCur == '\0') {
        /* empty buffer */
        s_ATBufferCur = s_ATBuffer;
        *s_ATBufferCur = '\0';
        p_read = s_ATBuffer;
    } else {   /* *s_ATBufferCur != '\0' */
        /* there's data in the buffer from the last read */

        // skip over leading newlines
        while (*s_ATBufferCur == '\r' || *s_ATBufferCur == '\n')
            s_ATBufferCur++;

        p_eol = findNextEOL(s_ATBufferCur);

        if (p_eol == NULL) {
            /* a partial line. move it up and prepare to read more */
            size_t len;

            len = strlen(s_ATBufferCur);

            memmove(s_ATBuffer, s_ATBufferCur, len + 1);
            p_read = s_ATBuffer + len;
            s_ATBufferCur = s_ATBuffer;
        }
        /* Otherwise, (p_eol !- NULL) there is a complete line  */
        /* that will be returned the while () loop below        */
    }

    while (p_eol == NULL) {
        if (0 == MAX_AT_RESPONSE - (p_read - s_ATBuffer)) {
            LOGE("ERROR: Input line exceeded buffer\n");
            /* ditch buffer and start over again */
            s_ATBufferCur = s_ATBuffer;
            *s_ATBufferCur = '\0';
            p_read = s_ATBuffer;
        }

        if(is_emulate)
        {
            count = read_emulate(p_read,
                                MAX_AT_RESPONSE - (p_read - s_ATBuffer));
            cur_cmd[0] = 0;
        }
        // cmy@20120105: for LG VL600
        else if(!modem_cmp(0x1004, 0x61AA, NULL))
        {
            memset(in_buf, 0, MAX_AT_RESPONSE);
            do {
                count = read(s_fd, in_buf, MAX_AT_RESPONSE);
                if(count > 0)
                    count = vl600_unpack(in_buf, count, p_read, MAX_AT_RESPONSE - (p_read - s_ATBuffer));
            } while (count < 0 && errno == EINTR);
        }
        else
        {
            do {
                count = read(s_fd, p_read,
                                MAX_AT_RESPONSE - (p_read - s_ATBuffer));
            } while (count < 0 && errno == EINTR);
        }

        if (count > 0) {
            AT_DUMP( "<< ", p_read, count );

            p_read[count] = '\0';

            // skip over leading newlines
            while (*s_ATBufferCur == '\r' || *s_ATBufferCur == '\n')
                s_ATBufferCur++;

            p_eol = findNextEOL(s_ATBufferCur);
            p_read += count;
        } else if (count <= 0) {
            /* read error encountered or EOF reached */
            if(count == 0) {
                LOGD("atchannel: EOF reached");
            } else {
                LOGD("atchannel: read error %s", strerror(errno));
            }
            return NULL;
        }
    }

    /* a full line in the buffer. Place a \0 over the \r and return */

    ret = s_ATBufferCur;
    *p_eol = '\0';
    s_ATBufferCur = p_eol + 1; /* this will always be <= p_read,    */
                              /* and there will be a \0 at *p_read */

    LOGD("AT< %s\n", ret);
    return ret;
}


static void onReaderClosed()
{
    if (s_onReaderClosed != NULL && s_readerClosed == 0) {

        pthread_mutex_lock(&s_commandmutex);

        s_readerClosed = 1;

        pthread_cond_signal(&s_commandcond);

        pthread_mutex_unlock(&s_commandmutex);

        s_onReaderClosed();
    }
}


static void *readerLoop(void *arg __unused)
{
    for (;;) {
        const char * line;

        line = readline();

        if (line == NULL) {
            break;
        }

        if(isSMSUnsolicited(line)) {
            char *line1;
            const char *line2;

            // The scope of string returned by 'readline()' is valid only
            // till next call to 'readline()' hence making a copy of line
            // before calling readline again.
            line1 = strdup(line);
            line2 = readline();

            if (line2 == NULL) {
                free(line1);
                break;
            }

            if (s_unsolHandler != NULL) {
                s_unsolHandler (line1, line2);
            }
            free(line1);
        } else {
            processLine(line);
        }
    }

    onReaderClosed();

    return NULL;
}

/**
 * Sends string s to the radio with a \r appended.
 * Returns AT_ERROR_* on error, 0 on success
 *
 * This function exists because as of writing, android libc does not
 * have buffered stdio.
 */
static int writeline (const char *s)
{
    size_t cur = 0;
    size_t len = strlen(s);
    ssize_t written;

    if (s_fd < 0 || s_readerClosed > 0) {
        return AT_ERROR_CHANNEL_CLOSED;
    }

    LOGD("AT> %s\n", s);

    AT_DUMP( ">> ", s, strlen(s) );

    if(is_emulate)
    {
        if(cur_cmd[0] == 0)
            strcpy(cur_cmd, s);
    }
    // cmy@20120105: for LG VL600
    else if(!modem_cmp(0x1004, 0x61AA, NULL))
    {
        memset(in_buf, 0, MAX_AT_RESPONSE);
        memset(out_buf, 0, MAX_AT_RESPONSE);
        sprintf(in_buf, "%s\r", s);
        len = vl600_pack(in_buf, out_buf, MAX_AT_RESPONSE);
        if(len <= 0) 
            return AT_ERROR_GENERIC;

        /* the main string */
        while (cur < len) {
            do {
                written = write (s_fd, out_buf + cur, len - cur);
            } while (written < 0 && errno == EINTR);

            if (written < 0) {
                return AT_ERROR_GENERIC;
            }

            cur += written;
        }

//        sleepMsec(500);
    }
    else
    {
        /* the main string */
        while (cur < len) {
            do {
                written = write (s_fd, s + cur, len - cur);
            } while (written < 0 && errno == EINTR);

            if (written < 0) {
                return AT_ERROR_GENERIC;
            }

            cur += written;
        }

        /* the \r  */

        do {
            written = write (s_fd, "\r" , 1);
        } while ((written < 0 && errno == EINTR) || (written == 0));

        if (written < 0) {
            return AT_ERROR_GENERIC;
        }
    }

    return 0;
}

static int writeCtrlZ (const char *s)
{
    size_t cur = 0;
    size_t len = strlen(s);
    ssize_t written;

    if (s_fd < 0 || s_readerClosed > 0) {
        return AT_ERROR_CHANNEL_CLOSED;
    }

    LOGD("AT> %s^Z\n", s);

    AT_DUMP( ">* ", s, strlen(s) );

    /* the main string */
    while (cur < len) {
        do {
            written = write (s_fd, s + cur, len - cur);
        } while (written < 0 && errno == EINTR);

        if (written < 0) {
            return AT_ERROR_GENERIC;
        }

        cur += written;
    }

    /* the ^Z  */

    do {
        written = write (s_fd, "\032" , 1);
    } while ((written < 0 && errno == EINTR) || (written == 0));

    if (written < 0) {
        return AT_ERROR_GENERIC;
    }

    return 0;
}

static void clearPendingCommand()
{
    if (sp_response != NULL) {
        at_response_free(sp_response);
    }

    sp_response = NULL;
    s_responsePrefix = NULL;
    s_smsPDU = NULL;
}


/**
 * Starts AT handler on stream "fd'
 * returns 0 on success, -1 on error
 */
int at_open(int fd, ATUnsolHandler h)
{
    int ret;
    pthread_t tid;
    pthread_attr_t attr;

    s_fd = fd;
    s_unsolHandler = h;
    s_readerClosed = 0;

    s_responsePrefix = NULL;
    s_smsPDU = NULL;
    sp_response = NULL;

    /* Android power control ioctl */
#ifdef HAVE_ANDROID_OS
#ifdef OMAP_CSMI_POWER_CONTROL
    ret = ioctl(fd, OMAP_CSMI_TTY_ENABLE_ACK);
    if(ret == 0) {
        int ack_count;
		int read_count;
        int old_flags;
        char sync_buf[256];
        old_flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, old_flags | O_NONBLOCK);
        do {
            ioctl(fd, OMAP_CSMI_TTY_READ_UNACKED, &ack_count);
			read_count = 0;
            do {
                ret = read(fd, sync_buf, sizeof(sync_buf));
				if(ret > 0)
					read_count += ret;
            } while(ret > 0 || (ret < 0 && errno == EINTR));
            ioctl(fd, OMAP_CSMI_TTY_ACK, &ack_count);
         } while(ack_count > 0 || read_count > 0);
        fcntl(fd, F_SETFL, old_flags);
        s_readCount = 0;
        s_ackPowerIoctl = 1;
    }
    else
        s_ackPowerIoctl = 0;

#else // OMAP_CSMI_POWER_CONTROL
    s_ackPowerIoctl = 0;

#endif // OMAP_CSMI_POWER_CONTROL
#endif /*HAVE_ANDROID_OS*/

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&s_tid_reader, &attr, readerLoop, &attr);

    if (ret < 0) {
        perror ("pthread_create");
        return -1;
    }


    return 0;
}

/* FIXME is it ok to call this from the reader and the command thread? */
void at_close()
{
    if (s_fd >= 0) {
        close(s_fd);
    }
    s_fd = -1;

    pthread_mutex_lock(&s_commandmutex);

    s_readerClosed = 1;

    pthread_cond_signal(&s_commandcond);

    pthread_mutex_unlock(&s_commandmutex);

    /* the reader thread should eventually die */
}

static ATResponse * at_response_new()
{
    return (ATResponse *) calloc(1, sizeof(ATResponse));
}

void at_response_free(ATResponse *p_response)
{
    ATLine *p_line;

    if (p_response == NULL) return;

    p_line = p_response->p_intermediates;

    while (p_line != NULL) {
        ATLine *p_toFree;

        p_toFree = p_line;
        p_line = p_line->p_next;

        free(p_toFree->line);
        free(p_toFree);
    }

    free (p_response->finalResponse);
    free (p_response);
}

/**
 * The line reader places the intermediate responses in reverse order
 * here we flip them back
 */
static void reverseIntermediates(ATResponse *p_response)
{
    ATLine *pcur,*pnext;

    pcur = p_response->p_intermediates;
    p_response->p_intermediates = NULL;

    while (pcur != NULL) {
        pnext = pcur->p_next;
        pcur->p_next = p_response->p_intermediates;
        p_response->p_intermediates = pcur;
        pcur = pnext;
    }
}

/**
 * Internal send_command implementation
 * Doesn't lock or call the timeout callback
 *
 * timeoutMsec == 0 means infinite timeout
 */

static int at_send_command_full_nolock (const char *command, ATCommandType type,
                    const char *responsePrefix, const char *smspdu,
                    long long timeoutMsec, ATResponse **pp_outResponse)
{
    int err = 0;
    struct timespec ts;

    if(sp_response != NULL) {
        err = AT_ERROR_COMMAND_PENDING;
        goto error;
    }

    //sleepMsec(DELAY_BEFORE_AT_CMD_MS);
    err = writeline (command);

    if (err < 0) {
        goto error;
    }

    s_type = type;
    s_responsePrefix = responsePrefix;
    s_smsPDU = smspdu;
    sp_response = at_response_new();

    if (timeoutMsec != 0) {
        setTimespecRelative(&ts, timeoutMsec);
    }

    while (sp_response->finalResponse == NULL && s_readerClosed == 0) {
        if (timeoutMsec != 0) {
            err = pthread_cond_timedwait(&s_commandcond, &s_commandmutex, &ts);
        } else {
            err = pthread_cond_wait(&s_commandcond, &s_commandmutex);
        }

        if (err == ETIMEDOUT) {
            err = AT_ERROR_TIMEOUT;
            goto error;
        }
    }

    if (pp_outResponse == NULL) {
        at_response_free(sp_response);
    } else {
        /* line reader stores intermediate responses in reverse order */
        reverseIntermediates(sp_response);
        *pp_outResponse = sp_response;
    }

    sp_response = NULL;

    if(s_readerClosed > 0) {
        err = AT_ERROR_CHANNEL_CLOSED;
        goto error;
    }

    err = 0;
error:
    clearPendingCommand();

    return err;
}

/**
 * Internal send_command implementation
 *
 * timeoutMsec == 0 means infinite timeout
 */
static int at_send_command_full (const char *command, ATCommandType type,
                    const char *responsePrefix, const char *smspdu,
                    long long timeoutMsec, ATResponse **pp_outResponse)
{
    int err;
    bool inEmulator;

    if (0 != pthread_equal(s_tid_reader, pthread_self())) {
        /* cannot be called from reader thread */
        return AT_ERROR_INVALID_THREAD;
    }
    inEmulator = isInEmulator();
    if (inEmulator) {
        pthread_mutex_lock(&s_writeMutex);
    }
    pthread_mutex_lock(&s_commandmutex);

    err = at_send_command_full_nolock(command, type,
                    responsePrefix, smspdu,
                    timeoutMsec, pp_outResponse);

    pthread_mutex_unlock(&s_commandmutex);
    if (inEmulator) {
        pthread_mutex_unlock(&s_writeMutex);
    }

    if (err == AT_ERROR_TIMEOUT && s_onTimeout != NULL) {
        s_onTimeout();
    }

    return err;
}


/**
 * Issue a single normal AT command with no intermediate response expected
 *
 * "command" should not include \r
 * pp_outResponse can be NULL
 *
 * if non-NULL, the resulting ATResponse * must be eventually freed with
 * at_response_free
 */
int at_send_command_t(const char *command, long long timeoutMsec, ATResponse **pp_outResponse)
{
    return at_send_command(command, pp_outResponse);
}

int at_send_command (const char *command, ATResponse **pp_outResponse)
{
    int err;

    err = at_send_command_full (command, NO_RESULT, NULL,
                                    NULL, 0, pp_outResponse);

    return err;
}


int at_send_command_singleline_t (const char *command,
                                const char *responsePrefix,
                                long long timeoutMsec,
                                 ATResponse **pp_outResponse)
{
    return at_send_command_singleline(command, responsePrefix, pp_outResponse);
}

int at_send_command_singleline (const char *command,
                                const char *responsePrefix,
                                 ATResponse **pp_outResponse)
{
    int err;

    err = at_send_command_full (command, SINGLELINE, responsePrefix,
                                    NULL, 0, pp_outResponse);

    if (err == 0 && pp_outResponse != NULL
        && (*pp_outResponse)->success > 0
        && (*pp_outResponse)->p_intermediates == NULL
    ) {
        /* successful command must have an intermediate response */
        at_response_free(*pp_outResponse);
        *pp_outResponse = NULL;
        return AT_ERROR_INVALID_RESPONSE;
    }

    return err;
}


int at_send_command_numeric (const char *command,
                                 ATResponse **pp_outResponse)
{
    int err;

    err = at_send_command_full (command, NUMERIC, NULL,
                                    NULL, 0, pp_outResponse);

    if (err == 0 && pp_outResponse != NULL
        && (*pp_outResponse)->success > 0
        && (*pp_outResponse)->p_intermediates == NULL
    ) {
        /* successful command must have an intermediate response */
        at_response_free(*pp_outResponse);
        *pp_outResponse = NULL;
        return AT_ERROR_INVALID_RESPONSE;
    }

    return err;
}


int at_send_command_sms (const char *command,
                                const char *pdu,
                                const char *responsePrefix,
                                 ATResponse **pp_outResponse)
{
    int err;

    err = at_send_command_full (command, SINGLELINE, responsePrefix,
                                    pdu, 0, pp_outResponse);

    if (err == 0 && pp_outResponse != NULL
        && (*pp_outResponse)->success > 0
        && (*pp_outResponse)->p_intermediates == NULL
    ) {
        /* successful command must have an intermediate response */
        at_response_free(*pp_outResponse);
        *pp_outResponse = NULL;
        return AT_ERROR_INVALID_RESPONSE;
    }

    return err;
}


int at_send_command_multiline_t (const char *command,
                                const char *responsePrefix,
                                long long timeoutMsec,
                                 ATResponse **pp_outResponse)
{
    return at_send_command_multiline(command, responsePrefix, pp_outResponse);
}

int at_send_command_multiline (const char *command,
                                const char *responsePrefix,
                                 ATResponse **pp_outResponse)
{
    int err;

    err = at_send_command_full (command, MULTILINE, responsePrefix,
                                    NULL, 0, pp_outResponse);

    return err;
}


/** This callback is invoked on the command thread */
void at_set_on_timeout(void (*onTimeout)(void))
{
    s_onTimeout = onTimeout;
}

/**
 *  This callback is invoked on the reader thread (like ATUnsolHandler)
 *  when the input stream closes before you call at_close
 *  (not when you call at_close())
 *  You should still call at_close()
 */

void at_set_on_reader_closed(void (*onClose)(void))
{
    s_onReaderClosed = onClose;
}

/**
 * Periodically issue an AT command and wait for a response.
 * Used to ensure channel has start up and is active
 */
int at_handshake()
{
    int i;
    int err = 0;
    bool inEmulator;

    if (0 != pthread_equal(s_tid_reader, pthread_self())) {
        /* cannot be called from reader thread */
        return AT_ERROR_INVALID_THREAD;
    }
    inEmulator = isInEmulator();
    if (inEmulator) {
        pthread_mutex_lock(&s_writeMutex);
    }
    pthread_mutex_lock(&s_commandmutex);

    for (i = 0 ; i < HANDSHAKE_RETRY_COUNT ; i++) {
        /* some stacks start with verbose off */
        err = at_send_command_full_nolock ("ATE0", NO_RESULT,
                    NULL, NULL, HANDSHAKE_TIMEOUT_MSEC, NULL);

        if (err == 0) {
            break;
        }
    }

    if (err == 0) {
        /* pause for a bit to let the input buffer drain any unmatched OK's
           (they will appear as extraneous unsolicited responses) */

        sleepMsec(HANDSHAKE_TIMEOUT_MSEC);
    }

    pthread_mutex_unlock(&s_commandmutex);
    if (inEmulator) {
        pthread_mutex_unlock(&s_writeMutex);
    }

    return err;
}


/**
 * Returns error code from response
 * Assumes AT+CMEE=1 (numeric) mode
 */
AT_CME_Error at_get_cme_error(const ATResponse *p_response)
{
    int ret;
    int err;
    char *p_cur;

    if (p_response->success > 0) {
        return CME_SUCCESS;
    }

    if (p_response->finalResponse == NULL
        || !strStartsWith(p_response->finalResponse, "+CME ERROR:")
    ) {
        return CME_ERROR_NON_CME;
    }

    p_cur = p_response->finalResponse;
    err = at_tok_start(&p_cur);

    if (err < 0) {
        return CME_ERROR_NON_CME;
    }

    err = at_tok_nextint(&p_cur, &ret);

    if (err < 0) {
        return CME_ERROR_NON_CME;
    }

    return (AT_CME_Error) ret;
}

