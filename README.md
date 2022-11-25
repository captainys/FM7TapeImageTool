# FM-7 Tape Image Tool
## by CaptainYS (http://www.ysflight.com)
---

# Introduction
This program makes a .T77 tape image from 44.1KHz WAV sampling of FM-7 data (program) tape.

.T77 format file records duration of negative phases and positive phases of the wave.  However, the raw WAV recording typically have waves that are not appropriate for extracting information needed for .T77 format data.

For example, very often some waves are shifted to plus side or minus side.  If the wave is shifted to the plus side, the duration of the minus phase gets shorter and plus phase gets longer.

This program first applies sequence of the filters to re-shape the wave so that the necessary information can be more accurately extracted, and then recognize data bytes that FM-7 BIOS recognizes, and then re-encode the recognized data bytes to .T77 format file.

このプログラムは、44.1KHzで録音したWAV形式ファイルから、FM-7/77/77AVエミュレータで使用可能な.T77形式のテープイメージファイルを作成します。

.T77形式ファイルは、波がプラス側、マイナス側に振れている時間を記録します。しかし、WAV形式で録音した波は必ずしもそのデータの抜き出しに適していません。

例えば、多くの場合、波がプラス側かマイナス側にシフトしていることがあります。例えば、波がプラス側にシフトしていた場合、マイナス側のフェーズは短くなり、プラス側のフェーズが長くなります。

このプログラムは、最初にフィルターをかけて、波の形を整えて、正しい時間を計測しやすくします。その上で、FM-7のBIOSが認識するのと同じデータを抜き出して、そのデータを再エンコードすることで.T77形式ファイルを作成します。




# Tips for Successful Imaging
Data/Program tapes are surprisingly durable.  I have a better success ratio of imaging tapes than imaging disks.  However, it is not as simple as reading and recording sectors.  There are some tips for successful imaging.

First, get a working data recorder.  Audio cassette players apply strange filter in order to make sound better for human ears, which is a bad idea for computer data.

A working data recorder is a must, although not easy to find.  I bought two data recorders from Yahoo! Auction.  None worked.  One said the motor worked.  It did, but no audio came out.  The second one the motor cannot play at constant speed.  Luckily one of my friends let me use his data recorder and said I can borrow it forever.  So, I am keeping it for the time being.  It is pricy, but I suggest to get one from a reliable retro source like Beep.

Then, use a light-weight WAV-recording program.

Audacity is very popular.  However, it is heavy and drops too many waves.  Missing one wave may not be audible to human ears.  But, it means one lost bit.  Losing one bit in a binary data kills the whole data.  Therefore I do not recommend Audacity for this purpose.

I have been using a free program called Lock On.  Which never dropped a single wave.

Record FM-7 data/program tape stereo.  Don't force it to be monaural.  In my experience, left and right channels came with a small lag, but long enough to destroy waves when combined.  Record both left and right channels.  If you use a good data recorder, you probably see audio in only one of the two channels, which is good.

Seriously adjust the volume .  Do not saturate, but not too low volume.  I shoot for 70% of the maximum level.
