# BSUCampusNetwork

北京体育大学校园网自动连接与登录工具。

## 项目简介

BSUCampusNetwork 用于自动完成北京体育大学校园网络的连接与登录。

1. 获取当前连接的 Wi-Fi
2. 自动连接 `BSU` Wi-Fi
3. 检查校园网是否已经认证
4. 自动登录校园网
5. Windows 开机自动启动


### BSUCampusNetworkClient

主程序。

负责：

* 获取当前 Wi-Fi
* 连接 BSU
* 检查校园网状态
* 登录校园网

注册表位置：

```text
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run
```

在项目根目录执行：

```powershell
cmake -S . -B build
```

构建 Release：

```powershell
cmake --build build --config Release
```

## 使用

### 启动客户端

```powershell
.\build\Release\BSUCampusNetworkClient.exe
```

### 安装开机启动

```powershell
.\build\Release\BSUCampusNetworkInstaller.exe
```

### 删除开机启动

```powershell
.\build\Release\BSUCampusNetworkUninstaller.exe
```

---