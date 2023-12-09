# arduino-miio

通过串口与小米模组通讯。

## 开发

推荐使用 [arduino-cli](https://arduino.github.io/arduino-cli/)。

### 依赖

```shell
# 配置开发板
arduino-cli config set board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32
```

### 编译

`arduino-cli compile --library . --fqbn esp32:esp32:esp32 ./examples/Connect/Connect.ino`

### 上传

根据设备情况指定端口。

`arduino-cli upload --library . --fqbn esp32:esp32:esp32 -p COM4 ./examples/Connect/Connect.ino`
