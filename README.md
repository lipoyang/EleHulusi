# えれふるす (FM音源ウインドシンセ)

M5StackとヤマハのFM音源チップを使った吹奏楽器です。リコーダーに似た指使いで簡単に吹くことができます。バグパイプにも似た持続音をともなって豊かなハーモニーを奏でるのが特長です。

EleHulusi is a wind instrument with M5Stack and YAMAHA FM synthesizer chip. It is easy to play like a recorder. It makes rich harmoniy with sustained sounds like a bagpipe.

## システム構成
![構成図](image/overview.png)
- M5Stack Basic V2.7 (Battery Bottom は不使用)
- YAMAHA FM音源LSI YMF825搭載モジュール基板
- ゲージ圧センサ MIS-2500-015G(5V) 
- Kailh Choc V1 ロープロファイルキースイッチ (赤軸)
- MBK Choc ロープロファイルキーキャップ
- 16bit I2C I/OエキスパンダIC MCP23017
- DC/DCコンバータ イーター電機工業 AS6R0-0505 (入力4.5～9V, 出力:5V/1A) 
- スピーカ 8Ω 2W 28mm角 LF-K2828B112A

## コンセプト

## 運指表
![構成図](image/finger.png)
- ソプラノリコーダーのドレミファソラシドの指使いを踏襲
- 半音の指使いはやや改変しています。特にC#4は変則的。
- A3～B3も出せるように変則的な指使いを定めています。
- 左手親指を開けると単純に1オクターブ上がります。

## ソフトウェア
- PlatformIOで開発 (Arduinoベース)
- 依存ライブラリ
    - M5Stack
    - Adafruit MCP23017 Arduino Library
    - SimpleYMF815 (自作ライブラリ)


