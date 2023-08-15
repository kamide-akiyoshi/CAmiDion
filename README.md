# 電子楽器CAmiDion
Arduino互換機で動作する電子楽器CAmiDionのソースコードです。

## 必要なライブラリ(Required library to compile)
使用する機能に応じて下記のライブラリが必要になります。
- 音源：[PWMDAC_Synth](https://github.com/kamide-akiyoshi/CAmiDion/)
- MIDI入出力：[Arduino MIDI library](https://www.arduino.cc/reference/en/libraries/midi-library/)
- 液晶表示（接続方法に応じて下記のいずれかを選択）
  - I<sup>2</sup>C（標準的な構成：[SinglePCB](https://camidion.wordpress.com/camidion/lineup/camidion-singlepcb/)や[2号機](https://camidion.wordpress.com/camidion/lineup/camidion-2/)相当）：[I2CLiquidCrystal](https://n.mtng.org/ele/arduino/i2c.html)
  - 74HC164経由([3号機](https://camidion.wordpress.com/camidion/lineup/camidion-3/)相当)：[Lcd74HC164](http://100year.cocolog-nifty.com/blog/2012/05/arduinolcd3-ide.html)
### コンパイル上の注意点
I2CLiquidCrystalに含まれているmglcd.hでは、#include <Arduino.h> が誤って #include <arduino.h> と記述されているために、
ファイル名の大文字小文字を区別していないWindowsでコンパイルが通っても、Linuxではこの誤りを直さないとコンパイルが通りませんので注意してください。

## 関連情報
その他の情報については下記を参照。
- 個人HP(on RIMNET) http://www.yk.rim.or.jp/~kamide/music/chordhelper/hardware/
- OSDN(こちらからリポジトリをGitHubへ引っ越しました) https://ja.osdn.net/users/kamide/pf/CAmiDion/ 
- ブログ https://camidion.wordpress.com/camidion/
