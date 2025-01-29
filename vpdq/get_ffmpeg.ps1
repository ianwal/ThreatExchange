$url = "https://github.com/GyanD/codexffmpeg/releases/download/7.1/ffmpeg-7.1-full_build-shared.zip"
Invoke-WebRequest -Uri $url -OutFile ffmpeg1.zip -UseBasicParsing

Expand-Archive -Path ffmpeg1.zip -DestinationPath cpp/

# CMake expects the folder to be named "ffmpeg", so rename it.
Rename-Item -Path "cpp/ffmpeg-7.1-full_build-shared" -NewName "ffmpeg"
