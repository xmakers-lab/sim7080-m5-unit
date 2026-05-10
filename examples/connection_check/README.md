# SIM7080 M5Stack Connection Check

## Overview
SIM7080G + M5Stack Basic の接続確認サンプルです。

以下を確認できます：
- モジュール応答
- SIM認識
- 電波強度（RSSI表示）
- キャリア接続
- 時刻取得（ネットワーク同期）

## Hardware
- M5Stack Basic
- SIM7080G Unit

## Wiring
ESP32 RX=GPIO22 (SIM7080 TX)  
ESP32 TX=GPIO21 (SIM7080 RX)

## Usage
