#
#  Copyright (c) 2015, The Linux Foundation. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#       * Redistributions of source code must retain the above copyright
#         notice, this list of conditions and the following disclaimer.
#       * Redistributions in binary form must reproduce the above
#         copyright notice, this list of conditions and the following
#         disclaimer in the documentation and/or other materials provided
#         with the distribution.
#       * Neither the name of The Linux Foundation nor the names of its
#         contributors may be used to endorse or promote products derived
#         from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
#  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
#  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
#  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
#  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
#  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
#  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

if [ ! -d "vendor/google_clockwork" ]; then
        if [ -d "vendor/pdk/platina" ]; then
                unzip -q vendor/pdk/platina/platina-userdebug/platform/google-clockwork-service*.zip -d vendor
        elif [ -d "vendor/pdk/mini_aw_armv7a_neon" ]; then
                unzip -q vendor/pdk/mini_aw_armv7a_neon/mini_aw_armv7a_neon-userdebug/platform/google-clockwork-service*.zip -d vendor
        elif [ -d "vendor/pdk/lenok" ]; then
                unzip -q vendor/pdk/lenok/lenok-userdebug/platform/google-clockwork-service*.zip -d vendor
        elif [ -d "vendor/pdk/bass" ]; then
                unzip -q vendor/pdk/bass/bass-userdebug/platform/google-clockwork-service*.zip -d vendor
        elif [ -d "vendor/pdk/nemo" ]; then
                unzip -q vendor/pdk/nemo/nemo-userdebug/platform/google-clockwork-service*.zip -d vendor
        elif [ -d "vendor/pdk/sprat" ]; then
                unzip -q vendor/pdk/sprat/sprat-userdebug/platform/google-clockwork-service*.zip -d vendor
        elif [ -d "vendor/pdk/wren" ]; then
                        unzip -q vendor/pdk/wren/wren-userdebug/platform/google-clockwork-service*.zip -d vendor
        elif [ -d "vendor/pdk/anthias" ]; then
                unzip -q vendor/pdk/anthias/anthias-userdebug/platform/google-clockwork-service*.zip -d vendor
        elif [ -d "vendor/pdk/sturgeon" ]; then
                unzip -q vendor/pdk/sturgeon/sturgeon-userdebug/platform/google-clockwork-service*.zip -d vendor
fi

fi
XCOMP64=`pwd`/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.8/bin
if [ -d "$XCOMP64" ]; then
  XCOMP64_INPATH=$(echo "$PATH" | grep "$XCOMP64" | head -n1)
  if [ -z "$XCOMP64_INPATH" ]; then
    echo "Adding 64-bit toolchain to path ($XCOMP64)"
    export PATH="$PATH:$XCOMP64"
  fi
fi

