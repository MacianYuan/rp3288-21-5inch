/* Android AMTL
 *
 * Copyright (C) Intel 2015
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

package com.intel.amtl.modem.output_config;

import com.intel.amtl.exceptions.ModemControlException;
import com.intel.amtl.models.config.ModemConf;
import com.intel.amtl.modem.controller.ModemController;

public interface LogOutputConfig {
    public String confTraceAndModemInfo(ModemConf mdmConf, ModemController mdmCtrl)
            throws ModemControlException;
    public String checkAtXsioState(ModemController mdmCtrl) throws ModemControlException;
    public String getAtXsioState(ModemController mdmCtrl) throws ModemControlException;
    public String getOctDriverPath();
}
