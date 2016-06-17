
### HVMでもconsoleしたい
`xl console <DomU>` でHVMでもコンソールできるようになりたい
- DomUの設定ファイルに `serial = "pty"` を追加する
- DomUのGRUB設定にシリアル接続できるよう設定する:w
  - 別にDomU固有の設定ということではなくLinuxのシリアル接続ができるようにする
  ```
root@test43:~# cat /etc/default/grub
GRUB_DEFAULT=0
GRUB_TIMEOUT=5
GRUB_DISTRIBUTOR=`lsb_release -i -s 2> /dev/null || echo Debian`
GRUB_CMDLINE_LINUX_DEFAULT=""
GRUB_CMDLINE_LINUX="console=tty1 console=ttyS0,115200"

GRUB_TERMINAL="console serial"
GRUB_SERIAL_COMMAND="serial --speed=115200 --unit=0 --word=8 --parity=no --stop=1"
  ```
  - 上のGRUB設定をしておくとカーネルが勝手に `/sbin/agetty --keep-baud 115200 38400 9600 ttyS0 vt102` を実行してくれる
    - 手動でコマンドを叩いてもいける

### pciパススルー
- なんかchipset 5500はPCIパススルーの一部機能が足りない?
  - 機能限定で動かせば使える
  - xenの起動パラメータに `iommu=no-intremap` を追加する
  ```
GRUB_CMDLINE_XEN="dom0_mem=2048M,max:2048M com1=115200,8n1 console=com1,vga iommu=no-intremap"
GRUB_TERMINAL="console serial"
GRUB_CMDLINE_LINUX_XEN_REPLACE="console=hvc0 earlyprintk=xen"
  ```
  - lspciでPCIのIDを調べる
  ```
root@xen01:~# lspci | grep Ethernet
02:00.0 Ethernet controller: Intel Corporation 82571EB Gigabit Ethernet Controller (rev 06)
02:00.1 Ethernet controller: Intel Corporation 82571EB Gigabit Ethernet Controller (rev 06)
05:00.0 Ethernet controller: Intel Corporation 82571EB Gigabit Ethernet Controller (rev 06)
05:00.1 Ethernet controller: Intel Corporation 82571EB Gigabit Ethernet Controller (rev 06)
08:00.0 Ethernet controller: Intel Corporation 82575EB Gigabit Network Connection (rev 02)
08:00.1 Ethernet controller: Intel Corporation 82575EB Gigabit Network Connection (rev 02)
  ```
  - こんな感じで追加する `xl pci-assignable-add 08:00.0`
  - `xl pci-assignable-list` で確認できる
  - domuのコンフィグに `pci = ['08:00.0']` を追加
    - dynamicにattach,detachできる
	```
root@xen01:~# xl pci-attach test42  '08:00.0,permissive=1'
root@xen01:~# xl pci-detach test42  '08:00.0,permissive=1'
	```
	- NICをブチブチ切れるけどよくなさそう...(もう一回attachしようとすると怒られる)

