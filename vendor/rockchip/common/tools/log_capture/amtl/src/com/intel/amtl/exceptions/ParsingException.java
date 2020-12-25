/* Android Modem Traces and Logs
 *
 * Copyright (C) Intel 2013
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
 *
 * Author: Morgane Butscher <morganeX.butscher@intel.com>
 */

package com.intel.amtl.exceptions;

import com.intel.amtl.log.AlogMarker;

public class ParsingException extends Exception {

    public ParsingException(String s) {
        super(s);
        AlogMarker.tAB("ParsingException.ParsingException", "0");
        AlogMarker.tAE("ParsingException.ParsingException", "0");
    }
}
