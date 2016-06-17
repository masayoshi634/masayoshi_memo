
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
