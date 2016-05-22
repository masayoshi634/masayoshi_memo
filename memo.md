# Consulのインスト〜る
- wgetでバイナリ落としてきてPATHが通っているディレクトリにポイ
  - バージョンが0.6まで上がってた(プロトコルのバージョンは2っぽい)
```
wget https://releases.hashicorp.com/consul/0.6.4/consul_0.6.4_linux_amd64.zip
unzip consul_0.6.4_linux_amd64.zip
mv consul /usr/bin/
```
# 基本動作確認
## server
- serverとして起動する
```
consul agent -server -bootstrap-expect 1 -data-dir /tmp/consul -client=0.0.0.0
```
- 8600がDNS,8500がhttp,8400がRPCっぽい
```
root@consulmaster01:~# netstat -natup | grep 'consul'
tcp6       0      0 :::8600                 :::*                    LISTEN      1627/consul
tcp6       0      0 :::8300                 :::*                    LISTEN      1627/consul
tcp6       0      0 :::8301                 :::*                    LISTEN      1627/consul
tcp6       0      0 :::8302                 :::*                    LISTEN      1627/consul
tcp6       0      0 :::8400                 :::*                    LISTEN      1627/consul
tcp6       0      0 :::8500                 :::*                    LISTEN      1627/consul
udp6       0      0 :::8600                 :::*                                1627/consul
udp6       0      0 :::8301                 :::*                                1627/consul
udp6       0      0 :::8302                 :::*                                1627/consul
```
- `consul members` でノードの一覧が見える
```
root@consulmaster01:~# consul members
Node            Address             Status  Type    Build  Protocol  DC
consulmaster01  192.168.10.60:8301  alive   server  0.6.4  2         dc1
```
- DNSも使える
  - `node名.node.consul` って名前で名前解決ができる
  - ポートは8600なのでポートを指定する必要あり(コンフィグファイルで設定できる)
  ```
▶ dig @192.168.10.60 -p 8600 consulmaster01.node.consul

; <<>> DiG 9.10.3-P4-Debian <<>> @192.168.10.60 -p 8600 consulmaster01.node.consul
; (1 server found)
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 60489
;; flags: qr aa rd; QUERY: 1, ANSWER: 1, AUTHORITY: 0, ADDITIONAL: 0
;; WARNING: recursion requested but not available

;; QUESTION SECTION:
;consulmaster01.node.consul.    IN      A

;; ANSWER SECTION:
consulmaster01.node.consul. 0   IN      A       192.168.10.60

;; Query time: 4 msec
;; SERVER: 192.168.10.60#8600(192.168.10.60)
;; WHEN: Sun May 22 00:21:37 JST 2016
;; MSG SIZE  rcvd: 86
  ```
  - ポートは設定変えられなさそう？自宅ではunboundのstub-zoneに設定している
  ```
stub-zone:
        name: "node.consul."
        stub-addr: 192.168.10.60@8600
  ```
- API
  - ノードの一覧がjsonで取れる
  - これ以外にもたくさんAPIがあるっぽい
```
 curl 192.168.10.60:8500/v1/catalog/nodes
[{"Node":"consulmaster01","Address":"192.168.10.60","TaggedAddresses":{"wan":"192.168.10.60"},"CreateIndex":3,"ModifyIndex":361},{"Node":"test07","Address":"192.168.10.202","TaggedAddresses":{"wan":"192.168.10.202"},"CreateIndex":373,"ModifyIndex":374}]% 
```

## client
- 各ノードで下記コマンドを実行すれば良さそう
```
consul agent -data-dir /tmp/consul -bind 0.0.0.0 -join 192.168.10.60
```
- `consul members` で増えたことが確認できる
```
root@test07:~# consul members
Node            Address              Status  Type    Build  Protocol  DC
consulmaster01  192.168.10.60:8301   alive   server  0.6.4  2         dc1
test07          192.168.10.202:8301  alive   client  0.6.4  2         dc1
```

# 設定ファイル書く
## /etc/consul.d/
- ディレクトリを作って設定を書く
  - json形式で設定がかけるぞ！
  - https://www.consul.io/docs/agent/options.html#configuration_files
  - serverは↓な感じにしてみた
  ```
{
  "datacenter":"home",
  "data_dir":"/var/consul/data",
  "ui_dir":"/var/consul/ui",
  "server":true,
  "domain": "consul.",
  "bootstrap_expect": 1,
  "bind_addr": "192.168.10.60",
  "client_addr": "0.0.0.0",
  "log_level":"INFO",
  "ports": {
    "https": 8080,
    "dns": 53
  }
}
  ```
  - clientはこんな感じ
  ```
{
  "datacenter":"home",
  "start_join": [ "192.168.10.60" ],
  "data_dir":"/var/consul/data",
  "bind_addr": "0.0.0.0",
  "client_addr": "0.0.0.0",
  "log_level":"INFO",
  "ports": {
    "https": 8080,
    "dns": 53
  }
}
  ```
  - 起動
    - `nohup consul agent --config-dir="/etc/consul.d" >> /var/log/consul.log &`
## オーケストレーション
- `consul exec` で全ノードにコマンドを実行できる
  - serviceを試したかったので下記コマンドでnginxをインストール
  ```
root@consulmaster01:~# consul exec apt-get -y install nginx
root@consulmaster01:~# consul exec 'hostname > /var/www/html/index.html'
  ```
  - 全サーバにnginxがインストールされ、アクセスするとホスト名が表示される
- 他にもnode単位やservice単位にできる
  - https://www.consul.io/docs/commands/exec.html

## サービス
- とりあえずnginxでやってみる
  - 設定ファイルはこんな感じ
  - includeを意識せず、ディレクトリにjsonファイルを追加すればいいだけなのは魅力
  - curlで15秒おきにチェック
  ```
cat << EOF > /etc/consul.d/web.json
{
  "service": {
    "name": "web",
    "port": 80,
    "check": {
      "script": "curl localhost:80 >/dev/null 2>&1",
      "interval": "1s"
    }
  }
}
EOF
  ```
- DNSで引けるようになる
```
▶ dig web.service.consul

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 4096
;; QUESTION SECTION:
;web.service.consul.            IN      A

;; ANSWER SECTION:
web.service.consul.     0       IN      A       192.168.10.202

;; Query time: 1 msec
;; SERVER: 192.168.10.1#53(192.168.10.1)
;; WHEN: Sun May 22 02:21:55 JST 2016
;; MSG SIZE  rcvd: 63

~
▶ dig web.service.consul SRV

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 4096
;; QUESTION SECTION:
;web.service.consul.            IN      SRV

;; ANSWER SECTION:
web.service.consul.     0       IN      SRV     1 1 80 test07.node.home.consul.

;; Query time: 1 msec
;; SERVER: 192.168.10.1#53(192.168.10.1)
;; WHEN: Sun May 22 02:22:01 JST 2016
;; MSG SIZE  rcvd: 90
```
- nginxを落とすとこのレコードから外れる
  - これを使ってDNSラウンドロビンによるロードバランシグを簡単に使える(実際に使うかは別にして)
  - 再びスタートしたらレコードに復活した

## lvs連携
- serviceにwebを作成し,Nginxを起動させる(上記のweb.json)
- ipvsを起動させたホストを用意する
- 同じセグメントでDRで設定(NICの設定など)
  - ↓な感じにした
```
root@lvskp01a:~# ipvsadm -Ln
IP Virtual Server version 1.2.1 (size=4096)
Prot LocalAddress:Port Scheduler Flags
  -> RemoteAddress:Port           Forward Weight ActiveConn InActConn
TCP  192.168.10.50:80 wrr
  -> 192.168.10.180:80            Route   1      0          0
  -> 192.168.10.188:80            Route   1      0          0
  -> 192.168.10.202:80            Route   1      0          0
```
- `consul watch -type=checks <実行するコマンド>` でhealthcheckなどで状態が変化したときにコマンドが実行される
  - 困ったことに変化しかわからない(死んだのか生き返ったのかわからない)
  - consulのreloadとかでも実行されてしまうし、べき等性が必須な感じ
  - さらに困ったことにserviceがwebでstateがCriticalみたいな積集合も取れない
  - なので下記雑スクリプトにより、NodesのリストとってきてServiceがwebでstateがCriticalな奴はweight0に
stateがpassingなものはweight1にするみたいな感じにした
- ipvsは超高速に設定が切り替わるので、切り替え時間はconsulのhealthcheck間隔に依存しそう(1sにしたら大体1秒で切り替わった)
  - persistentとかはどうなんだろうなぁ
  - 実行コマンドは `ipvsadm -e -t 192.168.10.50:80 -r [対象のRIP] -w [0 or 1]` みたいな感じ
```
import subprocess
import json

def get_passing_nodes():
        cmd = "consul watch -type=checks -state=passing"
        ret  =  subprocess.check_output( cmd.split(" ") )
        return json.loads(ret)

def get_critical_nodes():
        cmd = "consul watch -type=checks -state=critical"
        ret  =  subprocess.check_output( cmd.split(" ") )
        return json.loads(ret)

def get_web_service_nodes():
        cmd = "consul watch -type=service -service=web"
        ret  =  subprocess.check_output( cmd.split(" ") )
        return json.loads(ret)

def get_nodes():
        cmd = "consul watch -type=nodes"
        ret  =  subprocess.check_output( cmd.split(" ") )
        return json.loads(ret)

def change_weight_realserver(rip, weight):
        cmd = "ipvsadm -e -t 192.168.10.50:80 -r " + rip + " -w " + str(weight)
        print "exec: " + cmd
        ret  =  subprocess.check_output( cmd.split(" ") )

if __name__ == '__main__':
        passing_nodes = get_passing_nodes()
        critical_nodes = get_critical_nodes()
        web_service_nodes = get_web_service_nodes()
        nodes = get_nodes()
        web_service_passing_nodes = []
        web_service_critical_nodes = []

        for row in passing_nodes:
                if row["Name"] in "Service 'web' check":
                        web_service_passing_nodes.append(row["Node"])

        for row in critical_nodes:
                if row["Name"] in "Service 'web' check":
                        web_service_critical_nodes.append(row["Node"])

        for node in web_service_nodes:
                if node["Node"]["Node"] in web_service_critical_nodes:
                        print node["Node"]["Address"] + "weight 0"
                        change_weight_realserver(node["Node"]["Address"], 0)
                if node["Node"]["Node"] in web_service_passing_nodes:
                        print node["Node"]["Address"] + "weight 1"
                        change_weight_realserver(node["Node"]["Address"], 1)
```
- ホスト毎落ちるとダメだった...
  - hostがcriticalでもserviceがpassingな状態がありうる
  - 当然hostがpassingでserviceがcriticalの状態(プロセスが落ちてる)こともあるわけで...
  - どっちかがcriticalならweghit0とかにしないとダメっぽい
```
        "Checks": [
            {
                "Node": "test07",
                "CheckID": "serfHealth",
                "Name": "Serf Health Status",
                "Status": "critical",
                "Notes": "",
                "Output": "Agent not live or unreachable",
                "ServiceID": "",
                "ServiceName": ""
            },
            {
                "Node": "test07",
                "CheckID": "service:web",
                "Name": "Service 'web' check",
                "Status": "passing",
                "Notes": "",
                "Output": "",
                "ServiceID": "web",
                "ServiceName": "web"
            }
        ]
```

# 気がついたこと
- daemon化どうしようか？
  - daemonのオプションはなさそう
  - とりあえず雑に&つけてrc.localにコマンド記載で動かしている
  - daemontoolsでいい？(下記の件と合わせて考える必要あり)
- serverのエージェントが落ちて再起動した場合、clientが再接続してくれない？
  - 少なくてもserver再起動後は `consul members` にも追加されなかった
    - `bootstrap-expect` とか設定書かなきゃいけないのかな？自動ではできない？
  - serverでクラスタを組みことが前提だから全部落ちた時のことは考えてない？
  - serverがクラスタ組んでると再起動したら勝手にノード情報を同期してくれるのか？
    - 常識的に考えたらそれぐらいの機能はありそう

