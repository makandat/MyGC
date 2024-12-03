make -s
gcc -o bin/test_gc -std=c11 -g test_gc.c bin/my_gc.so
echo "更新されました。 bin/test_gc."
