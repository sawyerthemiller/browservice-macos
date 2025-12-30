## Browservice MacOS - Hosting from an AS Computer

Written with the help of Google Antigravity

1. run './setup_cef_mac.sh'
2. run 'cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release .'
3. run 'make -j$(sysctl -n hw.logicalcpu)'

4. Finally, to make an exectuable .app bundle, run './create_bundle.sh'

Enjoy and happy browsing :)
