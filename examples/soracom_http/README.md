# soracom_http

SIM7080 と M5Stack を使って、SORACOM Harvest にデータを送信するサンプルです。

## Overview

このサンプルでは、SIM7080G + M5Stack Basic を使ってクラウドへデータ送信する一連の流れを確認できます。

- モデム初期化（ボーレート自動復旧）
- モバイルネットワーク接続
- HTTP通信によるデータ送信
- LCDによる状態表示

connection_check の次のステップとして、
「接続確認 → データ送信」までを実現するサンプルです。

---

## Features

- TinyGSMライブラリ使用
- 30秒ごとの周期送信
- HTTP POSTによるデータ送信
- 汎用HTTP関数（他サービスにも応用可能）
- LCDログ表示
- 電波強度（RSSI）表示
- ネットワーク時刻表示
- ボーレート自動復旧（115200 / 9600）

---

## Hardware

- M5Stack Basic
- SIM7080G Unit
- SORACOM Air SIM
