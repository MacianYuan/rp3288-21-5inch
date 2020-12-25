/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>

#include <cutils/log.h>

#include "AkmSensor.h"
#include "akm8975.h"

/*****************************************************************************/

AkmSensor::AkmSensor()
: SensorBase(AKM_DEVICE_NAME, "compass"),
      mEnabled(0),
      mPendingMask(0),
      mInputReader(32)
{
    memset(mPendingEvents, 0, sizeof(mPendingEvents));
    memset(mMagnInsertingEvents, 0, sizeof(mMagnInsertingEvents));
    mPretimestamp = 0;
/*
    mPendingEvents[Accelerometer].version = sizeof(sensors_event_t);
    mPendingEvents[Accelerometer].sensor = ID_A;
    mPendingEvents[Accelerometer].type = SENSOR_TYPE_ACCELEROMETER;
    mPendingEvents[Accelerometer].acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
*/
    mPendingEvents[MagneticField].version = sizeof(sensors_event_t);
    mPendingEvents[MagneticField].sensor = ID_M;
    mPendingEvents[MagneticField].type = SENSOR_TYPE_MAGNETIC_FIELD;
    mPendingEvents[MagneticField].magnetic.status = SENSOR_STATUS_ACCURACY_HIGH;
/*
    mPendingEvents[Orientation  ].version = sizeof(sensors_event_t);
    mPendingEvents[Orientation  ].sensor = ID_O;
    mPendingEvents[Orientation  ].type = SENSOR_TYPE_ORIENTATION;
    mPendingEvents[Orientation  ].orientation.status = SENSOR_STATUS_ACCURACY_HIGH;
*/
    for (int i=0 ; i<numSensors ; i++)
        mDelays[i] = 200000000; // 200 ms by default

    // read the actual value of all sensors if they're enabled already
    struct input_absinfo absinfo;
    short flags = 0;

    open_device();
/*
    if (!ioctl(dev_fd, ECS_IOCTL_APP_GET_AFLAG, &flags)) {
        if (flags)  {
            mEnabled |= 1<<Accelerometer;
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_X), &absinfo)) {
                mPendingEvents[Accelerometer].acceleration.x = absinfo.value * CONVERT_A_X;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_Y), &absinfo)) {
                mPendingEvents[Accelerometer].acceleration.y = absinfo.value * CONVERT_A_Y;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_Z), &absinfo)) {
                mPendingEvents[Accelerometer].acceleration.z = absinfo.value * CONVERT_A_Z;
            }
        }
    }
*/	
    if ((dev_fd > 0) && (!ioctl(dev_fd, ECS_IOCTL_APP_GET_MVFLAG, &flags))) {
        if (flags)  {
            mEnabled |= 1<<MagneticField;
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAGV_X), &absinfo)) {
                mPendingEvents[MagneticField].magnetic.x = absinfo.value * CONVERT_M_X;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAGV_Y), &absinfo)) {
                mPendingEvents[MagneticField].magnetic.y = absinfo.value * CONVERT_M_Y;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAGV_Z), &absinfo)) {
                mPendingEvents[MagneticField].magnetic.z = absinfo.value * CONVERT_M_Z;
            }
        }
    }
/*
    if (!ioctl(dev_fd, ECS_IOCTL_APP_GET_MFLAG, &flags)) {
        if (flags)  {
            mEnabled |= 1<<Orientation;
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_YAW), &absinfo)) {
                mPendingEvents[Orientation].orientation.azimuth = absinfo.value;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_PITCH), &absinfo)) {
                mPendingEvents[Orientation].orientation.pitch = absinfo.value;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ROLL), &absinfo)) {
                mPendingEvents[Orientation].orientation.roll = -absinfo.value;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ORIENT_STATUS), &absinfo)) {
                mPendingEvents[Orientation].orientation.status = uint8_t(absinfo.value & SENSOR_STATE_MASK);
            }
        }
    }
*/
    // disable temperature sensor, since it is not reported
    flags = 0;
    if (dev_fd > 0)
        ioctl(dev_fd, ECS_IOCTL_APP_SET_TFLAG, &flags);
}

AkmSensor::~AkmSensor() {
    if (dev_fd > 0) {
        close(dev_fd);
        dev_fd = -1;
    }
}

int AkmSensor::enable(int32_t handle, int en)
{
    int what = -1;
    switch (handle) {
        //case ID_A: what = Accelerometer; break;
        case ID_M: what = MagneticField; break;
        //case ID_O: what = Orientation;   break;
    }

    if (uint32_t(what) >= numSensors)
        return -EINVAL;

    int newState  = en ? 1 : 0;
    int err = 0;

    if ((uint32_t(newState)<<what) != (mEnabled & (1<<what))) {
        if (dev_fd < 0) {
            open_device();
        }
        int cmd;
        switch (what) {
            //case Accelerometer: cmd = ECS_IOCTL_APP_SET_AFLAG;  break;
            case MagneticField: cmd = ECS_IOCTL_APP_SET_MVFLAG; break;
            //case Orientation:   cmd = ECS_IOCTL_APP_SET_MFLAG;  break;
        }
        short flags = newState;
        err = ioctl(dev_fd, cmd, &flags);
        err = err<0 ? -errno : 0;
        LOGE_IF(err, "ECS_IOCTL_APP_SET_XXX failed (%s)", strerror(-err));
        if (!err) {
            mEnabled &= ~(1<<what);
            mEnabled |= (uint32_t(flags)<<what);
            //update_delay();
        }
   	}

	return err;
}

int AkmSensor::setDelay(int32_t handle, int64_t ns)
{
#ifdef ECS_IOCTL_APP_SET_DELAY
    int what = -1;
    switch (handle) {
        //case ID_A: what = Accelerometer; break;
        case ID_M: what = MagneticField; break;
       // case ID_O: what = Orientation;   break;
    }

    if (uint32_t(what) >= numSensors)
        return -EINVAL;

    if (ns < 0)
        return -EINVAL;

    mDelays[what] = ns;
    return update_delay();
#else
    return -1;
#endif
}

int AkmSensor::update_delay()
{
    int result = 0;

    if (dev_fd < 0) {
        open_device();
    }

    uint64_t wanted = -1LLU;
    for (int i=0 ; i<numSensors ; i++) {
        if (mEnabled & (1<<i)) {
            uint64_t ns = mDelays[i];       /* 预期的 delay 设定. */
            wanted = wanted < ns ? wanted : ns;
        }
    }
    short delay = int64_t(wanted) / 1000000;
    if (ioctl(dev_fd, ECS_IOCTL_APP_SET_DELAY, &delay)) {
        return -errno;
    }

    return result;
}

int AkmSensor::isActivated(int  handle)
{
     int what = -1;
    switch (handle) {
        //case ID_A: what = Accelerometer; break;
        case ID_M: what = MagneticField; break;
       // case ID_O: what = Orientation;   break;
    }
    return (mEnabled & (1<<what));
}

int AkmSensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

    int numEventReceived = 0;       /* 已经接收的 event 的数量, 待返回. */
    input_event const* event;

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_ABS) {           // #define EV_ABS 0x03
            processEvent(event->code, event->value);
            mInputReader.next();
        } else if (type == EV_SYN) {    // #define EV_SYN 0x00
            for (int j=0 ; count && mPendingMask && j<numSensors ; j++) {
                if (mPendingMask & (1<<j)) {
                    mPendingMask &= ~(1<<j);
                    mPendingEvents[j].timestamp = getTimestamp();
                    if (mEnabled & (1<<j)) {
#ifdef INSERT_FAKE_DATA
                        if(mPretimestamp == 0)mPretimestamp = mPendingEvents[j].timestamp;
                        int tmstamp_ms =  nanoseconds_to_milliseconds(mPendingEvents[j].timestamp - mPretimestamp);
                        int num = tmstamp_ms/INSERT_DUR_MAX;
                        num -= tmstamp_ms%INSERT_DUR_MAX<INSERT_DUR_MIN? 1: 0;
                        num = num>=INSERT_FAKE_MAX ? 0 : num;
                        instertFakeData(num);
                        for(int k = 0;k<num;k++){
	                     *data++ = mMagnInsertingEvents[k];
	                     count--;
	                     numEventReceived++;
                        }
#endif
                        *data++ = mPendingEvents[j];
                        count--;
                        numEventReceived++;
                        mPretimestamp = mPendingEvents[j].timestamp;
                    }
                }
            }
            if (!mPendingMask) {
                mInputReader.next();
            }
        } else {
            LOGE("AkmSensor: unknown event (type=%d, code=%d)",
                    type, event->code);
            mInputReader.next();
        }
    }

    return numEventReceived;
}

void AkmSensor::instertFakeData(int num){
    for (int i=num-1; i>=0; i--){
	    mMagnInsertingEvents[i].version = mPendingEvents[MagneticField].version;
	    mMagnInsertingEvents[i].sensor = mPendingEvents[MagneticField].sensor;
	    mMagnInsertingEvents[i].type = mPendingEvents[MagneticField].type;
	    mMagnInsertingEvents[i].magnetic.status = mPendingEvents[MagneticField].magnetic.status;
           mMagnInsertingEvents[i].magnetic.x = mPendingEvents[MagneticField].magnetic.x;
           mMagnInsertingEvents[i].magnetic.y= mPendingEvents[MagneticField].magnetic.y;
           mMagnInsertingEvents[i].magnetic.z= mPendingEvents[MagneticField].magnetic.z;
           //usleep(5000);
           //mMagnInsertingEvents[i].timestamp = getTimestamp();
           mMagnInsertingEvents[i].timestamp = mPendingEvents[MagneticField].timestamp - INSERT_DUR_MAX*1000000*(num-i);
           D("hxw mMagnInsertingEvents[%d].timestamp:%ld\n",i,mMagnInsertingEvents[i].timestamp);
    }
}


void AkmSensor::processEvent(int code, int value)
{
    switch (code) {
/*
        case EVENT_TYPE_ACCEL_X:
            mPendingMask |= 1<<Accelerometer;
            mPendingEvents[Accelerometer].acceleration.x = value * CONVERT_A_X;
            break;
        case EVENT_TYPE_ACCEL_Y:
            mPendingMask |= 1<<Accelerometer;
            mPendingEvents[Accelerometer].acceleration.y = value * CONVERT_A_Y;
            break;
        case EVENT_TYPE_ACCEL_Z:
            mPendingMask |= 1<<Accelerometer;
            mPendingEvents[Accelerometer].acceleration.z = value * CONVERT_A_Z;
            break;
*/
        case EVENT_TYPE_MAGV_X:
            mPendingMask |= 1<<MagneticField;
            mPendingEvents[MagneticField].magnetic.x = value * CONVERT_M_X;
            break;
        case EVENT_TYPE_MAGV_Y:
            mPendingMask |= 1<<MagneticField;
            mPendingEvents[MagneticField].magnetic.y = value * CONVERT_M_Y;
            break;
        case EVENT_TYPE_MAGV_Z:
            mPendingMask |= 1<<MagneticField;
            mPendingEvents[MagneticField].magnetic.z = value * CONVERT_M_Z;
            break;
		case EVENT_TYPE_MAGV_STATUS:
            mPendingMask |= 1<<MagneticField;
            mPendingEvents[MagneticField].magnetic.status = value;
            break;
/*
        case EVENT_TYPE_YAW:
            mPendingMask |= 1<<Orientation;
            mPendingEvents[Orientation].orientation.azimuth = value * CONVERT_O_A;
            break;
        case EVENT_TYPE_PITCH:
            mPendingMask |= 1<<Orientation;
            mPendingEvents[Orientation].orientation.pitch = value * CONVERT_O_P;
            break;
        case EVENT_TYPE_ROLL:
            mPendingMask |= 1<<Orientation;
            mPendingEvents[Orientation].orientation.roll = value * CONVERT_O_R;
            break;
        case EVENT_TYPE_ORIENT_STATUS:
            mPendingMask |= 1<<Orientation;
            mPendingEvents[Orientation].orientation.status =
                    uint8_t(value & SENSOR_STATE_MASK);
            break;
*/
    }
}
