### vyos
- install
  - ユーザガイド通りにインストール
- 初期設定
  - ip address
  - nameserver

### Debianパッケージを使えるようにする
- squeezeベースなのでsqueezeを入れる
```
set system package repository squeeze components 'main contrib non-free'
set system package repository squeeze distribution 'squeeze'
set system package repository squeeze url 'http://archive.debian.org/debian'
set system package repository squeeze-lts components 'main contrib non-free'
set system package repository squeeze-lts distribution 'squeeze-lts'
set system package repository squeeze-lts url 'http://archive.debian.org/debian'
set system package repository squeeze-backports components main
set system package repository squeeze-backports distribution squeeze-backports
set system package repository squeeze-backports url 'http://backports.debian.org/debian-backports'
commit; save
```
- `sudo aptitude -o Acquire::Check-Valid-Until=false update`


### interface名を変更する
- インターフェースをdisableにする
  - `set interfaces ethernet eth0 disable`
- hw-idを削除する
  - `delete interfaces ethernet eth0 hw-id xx:xx:xx:xx:xx:xx`
  - `delete interfaces ethernet eth1 hw-id xx:xx:xx:xx:xx:xy`
  - `commit`
- hw-idをセットする
  - `set interfaces ethernet eth0 hw-id xx:xx:xx:xx:xx:xy`
  - `set interfaces ethernet eth1 hw-id xx:xx:xx:xx:xx:xx`
  - `commit`
- インターフェースをenableにする
  - `delete interfaces ethernet eth0 disable`
