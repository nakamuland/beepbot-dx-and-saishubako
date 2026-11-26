# beepBot DX & Saishubako for M5Stack Cardputer K132

このリポジトリには、標準 M5Stack Cardputer K132（ESP32-S3、8 MBフラッシュ、PSRAMなし）向けのランチャーと、beepBot DX／Saishubakoのソース一式を含みます。ランチャーから2つのアプリを切り替える試作構成です。

## 含まれるもの

- [`launcher/`](launcher/) — app0で動作し、選んだアプリをapp1へ書き込んで起動するランチャー
- [`beepbotdx/`](beepbotdx/) — beepBot DXのソース、テスト、画像。K132/no-PSRAMビルド設定とランチャー復帰対応を含み、元のMITライセンスを保持
- [`saishubako/`](saishubako/) — K132/no-PSRAM・OTA切り替え対応を加えたSaishubakoソース。元の画面回転設定とMITライセンスを保持
- [`tools/`](tools/) とルートのビルドスクリプト — 2つのアプリをビルドし、ランチャーに埋め込み、オフライン検査を行う補助ツール

## 起動構成

この試作は、標準の `default_8MB.csv` パーティション表にある2つのOTAアプリ領域を前提とします。

- **app0 (`0x10000`)**：ランチャー
- **app1 (`0x340000`)**：選んだアプリ。ランチャーがアプリイメージを書き込み、起動先を切り替えます。
- **Fn+Backspace**：beepBot DXからランチャーへ戻ります。未保存変更がある場合は確認画面でEnterを押すと保存せず破棄して戻り、Escでキャンセルします。beepBot DXの名前変更中はBackspaceとして動作します。
- **Fn+Q**：Saishubakoからランチャーへ戻ります。
- **Fn+P**：Saishubakoで保存済みメモを開いているとき、記録番号と本文をMX11へBluetooth LEで直接印刷します。

ランチャーから起動するとapp1の内容は選んだアプリで置き換わります。SaishubakoはmicroSDの `SAISHU` フォルダーを、beepBot DXは `/beepbotdx` を使用します。

## MX11へのメモ印刷

MX11に紙をセットし、iPhoneのFunPrintなど他のアプリとの接続を切ってから使います。
Saishubakoで `Fn+L` を押し、本文のあるメモを選んでEnterで開き、`Fn+P` を押します。
英数字・ひらがな・カタカナを白黒画像にして送ります。
録音だけで本文のない記録は、タイトルや説明を付けてから印刷してください。

実装は、単体版SAISHUBAKOと手元のMX11で印刷できた `51 78` / `AE01` のコマンド列を引き継いでいます。
同名のMX11には通信方式が異なる個体もあります。`Sent to MX11. Check paper.` は送信完了を示すため、実際の印字は紙で確認してください。
印刷中はスキャン・接続・送信の完了まで待ちます。`Fn+Q` の復帰操作は印刷処理が終わってから使えます。
接続できない場合は、iPhoneの設定でBluetoothをオフにしてMX11を入れ直してください。

統合版ではArduino core 3.2.0に合わせてNimBLE-Arduino 2.3.6を使用します。
単体版の実機確認と統合版での確認は、[変更記録](CHANGES.md)に記載します。

## ビルド

Python 3とPlatformIO CLIが必要です。依存するPlatformIOプラットフォーム／ライブラリの取得にはネットワーク接続が必要です。

2026-11-26に確認したクリーンビルド環境は、Ubuntu 24.04、Python 3.12.3、PlatformIO Core 6.2.0、Espressif 32 54.3.20、Arduino core 3.2.0です。他のOSやバージョンの組み合わせでは確認していません。

PlatformIOは `IRremote @ *` の候補が複数ある旨を警告し、この確認では `z3t0/IRremote@5.0.0` を選択しました。ビルドは成功しましたが、依存バージョンが完全固定されていないため、将来の依存解決が変わる可能性があります。

以下のコマンドはリポジトリのルートで実行してください。

LinuxまたはmacOS:

```sh
./build_all.sh
```

Windows PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File .\build_all.ps1
```

ビルド成果物は `dist/` に作られ、Git管理対象から除外されます。ビルドスクリプトはファームウェアを生成してオフライン検査を行うだけで、CardputerやmicroSDへの書き込みはしません。ランチャー用の `launcher/src/embedded_apps.S` もビルド時に生成されるファイルで、公開ソースには含めません。

## 実機への導入について

**このリポジトリにはフラッシュ書き込み用のインストーラーや、端末から吸い出したバックアップを含めません。** 実機へ導入する場合は、実機のパーティション表が上記の配置と一致することを読み取り専用で確認し、そのCardputer固有のバックアップを取ってください。app0の既存内容を置き換える操作になります。

## 実機確認

標準 Cardputer K132で、次の動作を確認しています。

- 再ビルドしたランチャーが横向き（landscape）で表示される。
- Saishubakoだけを回転させたアプリイメージを含むランチャーからSaishubakoが横向きで起動し、Fn+Qでランチャーへ戻る。
- beepBot DXは先行して実機確認済み。ランチャー内のbeepBot DXアプリイメージは、単体のK132向けbeepBot DXイメージとバイト単位で一致することを確認しています。

上記は表示・起動・復帰と埋め込みイメージの確認範囲です。Saishubakoの録音・再生や未保存データ保護など、ここに記していない機能まで確認済みとは主張しません。Cardputer ADVやCardputer Zero向けではありません。

## ライセンスと帰属

新規ランチャー／統合作業には、[ルートのMITライセンス](LICENSE)（Copyright (c) 2026 nakamuland）を適用します。beepBot DX（Brad McNally）とSaishubako（Saishubako contributors）の元のMITライセンス・著作権表示は各ディレクトリにそのまま保持しています。K132/no-PSRAM対応やランチャー復帰の変更によって、上流の帰属表示は置き換わりません。詳細は [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) を参照してください。依存ライブラリは各プロジェクトのライセンスに従います。
