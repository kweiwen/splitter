# splitter

## Windows 10

* create `external` folder to place the dependencies
```bash
mkdir external
```

* clone `eigen` to the `external` folder
```bash
cd external
git clone https://github.com/eigenteam/eigen-git-mirror.git eigen
cd eigen && git checkout 3.3.7 && cd -
```

* clone and build `spleeterpp` in the `external/spleeter/Release` folder
```bash
cd external
git clone https://github.com/gvne/spleeterpp.git && cd spleeterpp
git checkout 586a5dc503c73fc4975aca675432a280d6ea860f
mkdir build && cd build
mkdir Release && Release
cmake -Drtff_use_mkl=ON -DCMAKE_INSTALL_PREFIX=$(pwd)/install ..
cmake --build . --config Release --target INSTALL
```

* clone and build `spleeterpp` in the `external/spleeter/Debug` folder
```bash
cd external
git clone https://github.com/gvne/spleeterpp.git && cd spleeterpp
git checkout 586a5dc503c73fc4975aca675432a280d6ea860f
mkdir build && cd build
mkdir Debug && Debug
cmake -Drtff_use_mkl=ON -DCMAKE_INSTALL_PREFIX=$(pwd)/install ..
cmake --build . --config Debug --target INSTALL
```
