Dec2.dat (SimpleModulus server-decrypt keys)
============================================

Place the same binary as on the Takumi Android client: .../files/Data/Dec2.dat
(copy from device, data.zip, or a Windows client Data folder that matches your APK).

This directory is tracked in git so docker-compose can mount ./keys -> /keys/Dec2.dat
without each developer recreating ClientBuild_*/ paths.

After copying:
  cp /path/to/Dec2.dat ./Dec2.dat
  git add Dec2.dat

Legal / policy: Dec2.dat is derived from commercial MU client data; only commit if your
project and hosting policy allow distributing this key material.
