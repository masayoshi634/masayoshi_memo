# ipvsadmをぶっ放す！！！！！
## 設定コマンド
### 設定確認
- `ipvsadm -Ln` とかみんな知ってるやつ
  - ちなみに `/proc/net/ipvs` をみてもわかる(IPaddress,Portはわからん)
  ```
root@lvskp01a:~# cat /proc/net/ip_vs
IP Virtual Server version 1.2.1 (size=4096)
Prot LocalAddress:Port Scheduler Flags
  -> RemoteAddress:Port Forward Weight ActiveConn InActConn
TCP  C0A80A32:0050 wrr
  -> C0A80ABC:0050      Route   1      0          0
  -> C0A80ACA:0050      Route   1      0          0
  -> C0A80AB4:0050      Route   1      0          0
  ```
- よく見たら `persistent-conn` オプションとかある
- weight 0になってもpersistentが有効だとそっちにフォワーディングされるとかあるのでweight0にするときとかはつけたほうがいいかも
- `rate` オプションとかもあって意外と使えそうなのに使ってなかった
```
root@lvskp01a:~# ipvsadm  -Ln --persistent-conn
IP Virtual Server version 1.2.1 (size=4096)
Prot LocalAddress:Port            Weight    PersistConn ActiveConn InActConn
  -> RemoteAddress:Port
TCP  192.168.10.50:80 wrr
  -> 192.168.10.180:80            1         0           0          0
  -> 192.168.10.188:80            1         0           0          0
  -> 192.168.10.202:80            1         0           0          0
  ```
### 設定追加
- 大文字から始まるやつはVIP側の設定(大体)
- 小文字から始まるやつはRIP側の設定(大体)
- 以下な感じのを設定する(↑の設定確認で表示されている奴を作る)
  - 192.168.10.50がVIPは192.168.10.{180,188,202}でportはTCPの80番
- VIPの設定
  - `ipvsadm -A -t 192.168.10.50:80 -s wrr`
    - `A` が追加, `D` が削除, `E` が更新
    - `t` がTCP, `u` がUDP
    - `s` はスケジューラーで[rr|wrr|lc|wlc|lblc|lblcr|dh|sh|sed|nq]が選べる
- RIPの設定
  - `ipvsadm -a -t 192.168.10.50:80 -r 192.168.10.180 -g -w 1`
    - `g` はいわゆるDR,DSR,  `i` はいわゆるTUN,  `m` はいわゆるNAT(defaultはg)
      - これRIPごとに変えられるっぽい
	- `w` はweight
    - `a` が追加, `d` が削除, `e` が更新
      - `ipvsadm -e -t 192.168.10.50:80 -r 192.168.10.180 -g -w 0` とかでメンテナンスできるね!
## その他
- `strace ipvsadm -Ln` してたらsetsockoptしてるのが見えた
  - `/proc/net/ipvs` のIPアドレスを解決している感じっぽい
```
socket(PF_NETLINK, SOCK_RAW|SOCK_CLOEXEC, NETLINK_GENERIC) = 3
setsockopt(3, SOL_SOCKET, SO_SNDBUF, [32768], 4) = 0
setsockopt(3, SOL_SOCKET, SO_RCVBUF, [32768], 4) = 0
bind(3, {sa_family=AF_NETLINK, pid=14071, groups=00000000}, 12) = 0
getsockname(3, {sa_family=AF_NETLINK, pid=14071, groups=00000000}, [12]) = 0
sendmsg(3, {msg_name(12)={sa_family=AF_NETLINK, pid=0, groups=00000000}, msg_iov(1)=[{" \0\0\0\20\0\5\0\377\305AW\3676\0\0\3\1\0\0\t\0\2\0IPVS\0\0\0\0", 32}], msg_controllen=0, msg_flags= "}]})
```
- あとsetsockoptで `ip_vs_so_set_add` とかいうのを設定追加の時に指定してた
  - ipvsモジュールとやり取りするやつっぽい
- ファイル
  - /proc/net/ip_vs
  - /proc/net/ip_vs_app
  - /proc/net/ip_vs_conn
  - /proc/net/ip_vs_stats
  - /proc/sys/net/ipv4/vs/am_droprate
  - /proc/sys/net/ipv4/vs/amemthresh
  - /proc/sys/net/ipv4/vs/drop_entry
  - /proc/sys/net/ipv4/vs/drop_packet
  - /proc/sys/net/ipv4/vs/secure_tcp
  - /proc/sys/net/ipv4/vs/timeout_close
  - /proc/sys/net/ipv4/vs/timeout_closewait
  - /proc/sys/net/ipv4/vs/timeout_established
  - /proc/sys/net/ipv4/vs/timeout_finwait
  - /proc/sys/net/ipv4/vs/timeout_icmp
  - /proc/sys/net/ipv4/vs/timeout_lastack
  - /proc/sys/net/ipv4/vs/timeout_listen
  - /proc/sys/net/ipv4/vs/timeout_synack
  - /proc/sys/net/ipv4/vs/timeout_synrecv
  - /proc/sys/net/ipv4/vs/timeout_synsent
  - /proc/sys/net/ipv4/vs/timeout_timewait
  - /proc/sys/net/ipv4/vs/timeout_udp
