# syntax=docker/dockerfile:1
# Single image: self-contained LegacyLoginHost + GameHost + PostgreSQL (bookworm).
# Build from repo folder `server-next`:  docker compose -f docker-compose.all-in-one.yml build

FROM mcr.microsoft.com/dotnet/sdk:10.0 AS build
WORKDIR /src

ARG TARGETPLATFORM

COPY Takumi.Server.Next.slnx ./
COPY src/ ./src/

RUN set -eux; \
    case "${TARGETPLATFORM:-linux/amd64}" in \
        linux/arm64*) RID=linux-arm64 ;; \
        linux/amd64*) RID=linux-x64 ;; \
        *) echo "Unsupported TARGETPLATFORM=${TARGETPLATFORM:-<empty>}"; exit 1 ;; \
    esac; \
    echo "[build] TARGETPLATFORM=${TARGETPLATFORM:-?} -> RID=${RID}"; \
    dotnet publish src/Takumi.Server.LegacyLoginHost/Takumi.Server.LegacyLoginHost.csproj \
        -c Release -r "${RID}" --self-contained true -o /out/legacy; \
    dotnet publish src/Takumi.Server.GameHost/Takumi.Server.GameHost.csproj \
        -c Release -r "${RID}" --self-contained true -o /out/game

FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive \
    PGDATA=/var/lib/postgresql/data

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        tini \
        supervisor \
        postgresql \
        postgresql-client \
        libicu72 \
        libssl3 \
        zlib1g \
    && rm -rf /var/lib/apt/lists/*

# Debian puts cluster binaries under /usr/lib/postgresql/<ver>/bin/ — not on postgres user's PATH.
RUN set -eux; \
    PG_BIN="$(ls -d /usr/lib/postgresql/*/bin | sort -V | tail -n1)"; \
    test -x "${PG_BIN}/initdb"; \
    for x in initdb postgres pg_isready psql createdb; do \
      ln -sf "${PG_BIN}/${x}" "/usr/local/bin/${x}"; \
    done

ENV PATH="/usr/local/bin:/usr/local/sbin:/usr/sbin:/usr/bin:/sbin:/bin"

RUN mkdir -p /opt/takumi/legacy-login /opt/takumi/game-host \
    && mkdir -p /var/lib/postgresql/data \
    && mkdir -p /var/run /var/log/supervisor \
    && chown -R postgres:postgres /var/lib/postgresql/data

COPY --from=build /out/legacy/ /opt/takumi/legacy-login/
COPY --from=build /out/game/ /opt/takumi/game-host/

COPY deploy/all-in-one/entrypoint.sh /usr/local/bin/takumi-aio-entrypoint.sh
COPY deploy/all-in-one/supervisord.conf /etc/supervisor/supervisord.conf
COPY deploy/all-in-one/start-legacy.sh /usr/local/bin/takumi-start-legacy.sh
RUN chmod +x /usr/local/bin/takumi-aio-entrypoint.sh \
    && chmod +x /usr/local/bin/takumi-start-legacy.sh \
    && chmod +x /opt/takumi/legacy-login/Takumi.Server.LegacyLoginHost \
    && chmod +x /opt/takumi/game-host/Takumi.Server.GameHost

EXPOSE 5432 44605 44606 55901

ENTRYPOINT ["/usr/bin/tini", "-g", "--", "/usr/local/bin/takumi-aio-entrypoint.sh"]
