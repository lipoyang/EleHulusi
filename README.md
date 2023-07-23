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
- DC/DCコンバータ イーター電子 入力:～ 出力:5V/1A 
- スピーカー 8Ω 2W 28mm角

## ソフトウェア
- PlatformIOで開発 (Arduinoベース)
- 依存ライブラリ
    - M5Stack
    - Adafruit MCP23017 Arduino Library
    - SimpleYMF815 (自作ライブラリ)


