# black-library-binder
Black Library binding library

## How to build
```mkdir build```

and

```cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=install . -Bbuild```

or

```cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install . -Bbuild```

then

```make install -j`nproc` -Cbuild```