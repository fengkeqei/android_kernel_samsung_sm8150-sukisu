# android_kernel_samsung_sm8150-sukisu

三星 Galaxy S10 国行（SM-G9730 / beyond1qlte，SM8150）内核,
将 Linux 4.14.190 基线逐步合并 kernel.org 4.14.y 稳定版补丁至 EOL 版本 **4.14.336**，
每一步（207→217→…→317→336）均经真机刷入验证开机；
随后追加合并 **OpenELA linux-4.14.y 扩展维护至 4.14.357-openela**

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

