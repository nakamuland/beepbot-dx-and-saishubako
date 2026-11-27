# UnitCamS3と写真採集

対象は **Unit CamS3 U174 / OV2640 / 2MP版** です。PY260搭載の5MP版は対象外です。
カメラを初期化した後にセンサーIDも確認し、OV2640以外なら画像を返しません。

## 接続

1. 両方の電源を切ります。
2. UnitCamS3をPCとGrove2USB-Cから外します。書き込みにG0–GNDジャンパーを使った場合は外します。
3. 通常の4線GroveケーブルでCardputerの側面ポートとUnitCamS3を接続します。
4. Cardputerの電源を入れます。カメラはCardputerから給電されます。

| ケーブル | Cardputer | UnitCamS3 |
|---|---|---|
| 黒 | GND | GND |
| 赤 | 5V | 5V |
| 黄 | GPIO2 / RX | GPIO20 / TX |
| 白 | GPIO1 / TX | GPIO19 / RX |

Wi-Fi設定やインターネット接続は不要です。Grove端子を占有するため、同じポートのI2C機器との同時使用はできません。

## 操作

- ランチャーの **3 Camera**：カメラ専用画面。低解像度プレビューを約1秒間隔（撮影・転送時間を除く）で更新します。
- **Enter / G0**：1600×1200のJPEGをCardputerのmicroSDへ保存。保存後も続けて撮れます。
- **Tab / Fn+Q**：ランチャーへ戻ります。
- 採集箱の入力画面で **Fn+C**：写真を撮って入力画面へ戻ります。入力中のメモは保持し、Enterで保存すると写真と同じ記録番号になります。
- 採集箱の保存済み記録で **Fn+C**：写真があれば表示、なければその記録に写真を追加します。
- 写真表示中の **Tab**：記録本文へ戻ります。

写真は `/SAISHU/00000001.jpg` のように保存します。本文は同じ番号の `.txt`、録音は `.wav` です。記録一覧の `A` は録音、`P` は写真を表します。1記録につき写真1枚で、既存の写真は上書きしません。撮り直しは新しい記録として保存してください。
MX11へのFn+Pは引き続き本文の印刷です。写真の印刷は含みません。

## カメラ側のビルド・書き込み

```powershell
python -m platformio run -d unitcams3 -e unitcams3
python -m platformio run -d unitcams3 -e unitcams3 -t upload --upload-port COM21
```

COM番号は環境に合わせます。USBでの書き込みにはGrove2USB-Cを使用します。
通常起動後はGPIO19/20をUARTとして使い、USBの端子機能を解放するため、そのままUSB通信はできません。再書き込み時は電源を切り、G0–GNDを短絡してUSBを接続し、ダウンロードモードにします。書き込みが終わったら電源を切って短絡を外します。
出力は `.pio/build/unitcams3/` にあります。Cardputer用のBINをカメラへ書き込まないでください。

## 実装

UART 460800bps、8N1。プレビューは160×120 JPEG、写真は1600×1200 JPEGです。
カメラが256バイトごとに3ms待ち、PSRAMのない初代Cardputer側のSD書き込みを待ちます。撮影中は操作とプレビュー更新が止まり、数秒以上かかる場合があります。

- 要求：ASCII `SCQ1` と1バイトの `P`（プレビュー）または `S`（写真）。
- 応答：`SCJ1`、ステータス1バイト（0=成功）、予約3バイト、JPEG長LE32、JPEG全体のCRC32/IEEE LE32、JPEG本体。
- CRC初期値・最終XORは`0xffffffff`、反転多項式は`0xedb88320`です。
- Cardputerは長さ、CRC、JPEGの開始・終了マーカーを確認します。
- 写真は`.jpg.partial`に流し込み、検証後に`.jpg`へ変更します。失敗時は未完成ファイルを削除します。電源断で残った`.partial`は一覧に表示せず、番号を予約して上書きを防ぎます。
- カメラ専用画面と採集箱は同じアプリイメージを共有し、ランチャーがNVSへ起動モードを渡します。3つの大きなアプリをランチャーへ重複格納せず、既存の8MBパーティション構成を使います。

## 参照

- [M5Stack Unit CamS3公式仕様・配線・書き込み方法](https://docs.m5stack.com/en/unit/Unit_CamS3)
- [M5Stack Cardputer公式ピン配置](https://docs.m5stack.com/en/core/Cardputer)
- [Espressif esp32-camera](https://github.com/espressif/esp32-camera)

このプログラムはMITライセンスです。Arduino ESP32付属のカメラドライバを使用します。
