# Arduino Serial MIIO

使用 [小米 IoT 串口通讯协议](https://iot.mi.com/v2/new/doc/embedded-dev/module-dev/function-dev/serial-communication) 与小米 IoT 模组通讯。

## 使用

请参考 [示例](examples/Basic/Basic.ino)，以小米个人开发者模板设备 `perdev.switch.004` 为例。

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

`arduino-cli compile --library . --fqbn esp32:esp32:esp32 ./examples/Basic/Basic.ino`

### 上传

根据设备情况指定端口。

`arduino-cli upload --fqbn esp32:esp32:esp32 -p COM4 ./examples/Basic/Basic.ino`

### VSCode

`.vscode/arduino.json` 的内容：

```json
{
  "port": "COM5",
  "board": "esp32:esp32:esp32",
  "buildPreferences": [["build.defines", "-DDEBUG_ESP_PORT=Serial"]],
  "output": ".vscode/build",
  "sketch": "examples/Basic/Basic.ino"
}
```

使用 VSCode 配合 Arduino 插件开发时，没有找到设置当前工作目录为 library 的方法。

只能通过符号链接将项目所在文件夹放置 `arduino-cli.yaml` 配置文件中的 `directories.user` 中。（实现类似 pip install -e 的效果）

```pwsh
cd <directories.user>
New-Item -ItemType SymbolicLink -Path "SerialMIIO" -Target <arduino-miio 项目文件夹>
```

这样才能正确编译示例项目并生成 `c_cpp_properties.json`。

## 鸣谢

- 部分代码参考小米提供的 [示例代码](https://iot.mi.com/v2/new/doc/resources-and-services/personal-developer/embedded-dev#MCU%20程序开发)
