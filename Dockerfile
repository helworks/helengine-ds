FROM devkitpro/devkitarm:latest

ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITARM=${DEVKITPRO}/devkitARM
ENV LIBNDS=${DEVKITPRO}/libnds
ENV PATH=${DEVKITARM}/bin:${DEVKITPRO}/tools/bin:${PATH}

RUN dkp-pacman -Syu --noconfirm \
    && dkp-pacman -S --noconfirm nds-dev

WORKDIR /workspace
CMD ["/bin/bash"]
