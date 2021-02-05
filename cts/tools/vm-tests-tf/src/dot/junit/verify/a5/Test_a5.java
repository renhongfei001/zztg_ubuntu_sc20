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

package dot.junit.verify.a5;

import dot.junit.DxTestCase;
import dot.junit.DxUtil;

public class Test_a5 extends DxTestCase {

    /**
     * @constraint A5
     * 
     * @title The last instruction in the insns array must end at index insns_size-1.  
     */
    public void testVFE1() {
        try {
            Class.forName("dot.junit.verify.a5.d.T_a5_1");
            fail("expected a verification exception");
        } catch (Throwable t) {
            DxUtil.checkVerifyException(t);
        }
    }

}
