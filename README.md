# android_kernel_samsung_sm8150-sukisu

三星 Galaxy S10 国行（SM-G9730 / beyond1qlte，SM8150）内核，基于 SukiSU-Ultra v4.2.0 树，
将 Linux 4.14.190 基线逐步合并 kernel.org 4.14.y 稳定版补丁至 EOL 版本 **4.14.336**，
每一步（207→217→…→317→336）均经真机刷入验证开机；
随后追加合并 **OpenELA linux-4.14.y 扩展维护至 4.14.357-openela**（336→357 一次性合并，
1503 文件干净应用 + 54 冲突三方/手工缝合，产物待真机验证）。

> 本仓库为权威源：2026-09 的本地 `git-filter-repo` 事故后由 GitHub 同步恢复。

## 编译链

| 组件 | 版本 |
| --- | --- |
| Clang | 10.0.6（AOSP `llvm-arm-toolchain-ship-10.0`） |
| 交叉 binutils | NDK r16b `aarch64-linux-android-4.9`（binutils 2.27） |
| make | 4.3 |
| python2 | 必需（三星 RKP_CFP / FIPS 后处理脚本依赖） |

```bash
./build_beyond1qlte_sukisu_enforce.sh          # 增量编译
./build_beyond1qlte_sukisu_enforce.sh --clean  # 全量重编
```

- defconfig：`arch/arm64/configs/afaneh_beyond1qlte_defconfig`（KSU=y、kexec/kdump 关闭、SELinux 常开）
- 输出：`out-beyond1qlte-sukisu-191/arch/arm64/boot/Image-dtb`，日志 `build-sukisu-191.log`
- techpack 音频经 KCFLAGS 注入头文件与 `sm8150_beyondq.h`；techpack 报 E2BIG 时先删 `out*/techpack`
- 打包：`python3 KernelSource/work/pack_boot.py <boot_orig.img> <Image-dtb> <out.img>`
  （boot v1 模板替换内核载荷 + 重算 kernel_size/SHA1 id；boot_orig 需 ramdisk=0 的国行原厂 boot）

## 刷入注意事项

1. 设备进 TWRP，`adb push` 后 dd 写入 **boot 分区 `/dev/block/sda21`**，回读 sha256 与本地比对；
2. **绝对不能刷 dtbo**——fstab 在 dtbo 里，刷错直接无法开机；
3. KNOX warranty void 会导致偶发开机引导页重现、开发者模式被关、预装应用恢复，属正常现象；
4. 常备一个已验证可开机的 boot.img 以便 TWRP 恢复。

## 提交链（全部实测可开机）

```
0719367d8  基线（sukisu v4.2.0 / 4.14.190）
7d6fa61a3  合并到207 → f15de6199 217 → 26e332b5d 227 → 6cea0638a 237
526ab8e2c 247-core → 933c7351a 247+fs → e46c3e03e 247+fs+net → 0771bccad 247-近完整
98bcaa7de 257 → 17f5da2e2 267 → ac04c4bed 277 → eab5e7c9b 287（extcon.c 保持 277 规避 NULL deref）
48ebaac36 297 → 2d133267c 307 → dc584d3ad 317
d79978328  合并到336-EOL（4.14 最终版）
994e30004  336精修：18个冲突文件三方解析 + cred/events 手工适配（实测可开机）
49ee1a3d6  合并 OpenELA 337-357：1503 文件干净应用（SUBLEVEL=357）
d5ea0a8c5  337-357 冲突解析：54 文件三方合并/手工缝合   ← 当前 HEAD 源状态
```

## 目录速览

- `KernelSU/`：SukiSU-Ultra 内核态源（`drivers/kernelsu` 指向 `../KernelSU/kernel`）
- `techpack/`：高通音频外设代码（三星魔改 Makefile，需 KCFLAGS 注入）
- `drivers/net/wireless/broadcom/bcmdhd_101_16/`：Broadcom Wi-Fi
- 4.14.336 的 `LINUX_VERSION_CODE`(266064) 数值溢出到 4.15 区间，涉及 `>= 4.15` 守卫的
  外置驱动需按 4.14 语义评估（dhd 系列已处理）

## 已知边界

- kernel.org 4.14.y 已于 4.14.336（2024-01）EOL；OpenELA 的 linux-4.14.y 扩展维护也已于
  2024-12 结束（4.14.357 为其最终版本），此后无后续官方补丁。
