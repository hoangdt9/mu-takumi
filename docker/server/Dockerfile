# check=skip=FromPlatformFlagConstDisallowed
# MU Season Win32 servers — Wine trong container Linux amd64.
# Host mount: thư mục MuServer đầy đủ .exe + Data (build MSVC trên Windows).
#
# Apple Silicon + Docker: thường emulation QEMU → Wine hay crash. Xem docker/README.md .
# Build: env -u DOCKER_DEFAULT_PLATFORM docker compose build
#
FROM --platform=linux/amd64 scottyhardy/docker-wine:latest

USER root

WORKDIR /MuServer

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

# Win32 MU binaries; giảm spam log Wine khi chạy production-like
ENV DISPLAY=:99 \
    WINEARCH=win32 \
    WINEDEBUG=-all \
    WINEDLLOVERRIDES="mscoree,mshtml=" \
    WINEPREFIX=/wine

EXPOSE 44405/tcp 55901/tcp 55960/tcp 55557/tcp

ENTRYPOINT ["/entrypoint.sh"]
