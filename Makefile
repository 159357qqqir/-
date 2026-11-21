name: Build KPM Demo
on:
  push:
    branches: [ main ]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout
        uses: actions/checkout@v4
        with:
          submodules: true
      - name: Install aarch64-none-elf toolchain
        run: |
          sudo apt update
          sudo apt install -y gcc-aarch64-none-elf make
      - name: Build
        run: make
      - name: Upload artifact
        uses: actions/upload-artifact@v4
        with:
          path: demo.kpm
