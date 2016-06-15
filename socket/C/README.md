# 仮想ブリッジをC言語で作成する

###  遊び方

1. Linux(kernel 3.10以降)を用意
2. Cディレクトリ配下に移動して `make bridge`
3. `sudo ./script/init_testing.sh`
4. `sudo ./bridge veth1 veth2`
5. 別のターミナルで `sudo ip netns exec test01 ping 192.168.254.2`
6. pingが通り、 転送されたパケットのヘッダが表示されます
7. お片づけ `sudo ./script/del_testing.sh`
