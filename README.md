# Postcard simple protocol


## 1) Start python server

From root folder

```
cd python
python server_example.py
```

## 2) Start cpp client

From root folder

```
cd cpp
mkdir build
cd build
cmake ..
make
./client_example 127.0.0.1 8000 ../sample_image.jpg
```

or, on windows:

```
cd cpp
mkdir build
cd build
cmake ..
make
client_example.exe 127.0.0.1 8000 ..\sample_image.jpg
```

The CPP Client Example will connect to a Postcard Server running locally. It will send a "sample_image.jpg" to the server which will draw random circles over it and send back the modified version.
