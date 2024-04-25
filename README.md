# README

一个C++版本的AES-128算法实现，包括一个库`aes0.lib`和一个执行加密解密的可执行程序`aeszero.exe`。

`aeszero.exe`包括加密和解密两种模式，对应`-e`和`-d`选项。

在加密模式下，标准输入形式为
```
aeszero -e file_in -o file_out -k key
```
其中：
- `file_in`是输入文件名；（明文）
- `file_out`是输出文件名；（密文）
- `key`是密钥字符串（前16字节有效）

加密程序执行完成后，会自动导出一个名为`file_out.key`的混淆密钥文件（64字节），混淆密钥文件中含有真实的密钥字符串。


可以使用长度为64字节的混淆密钥文件替代密钥字符串进行输入，此时需要提供混淆密钥文件的文件名，例如
```
aeszero -e file_in -o file_out -f key_file
```

还可以直接不提供密钥或混淆密钥文件，此时程序会生成随机密钥
```
aeszero -e file_in -o file_out
```

在解密模式下，标准输入形式为
```
aeszero -d file_in -o file_out -k key
```
其中：
- `file_in`是输入文件名；（密文）
- `file_out`是输出文件名；（明文）
- `key`是密钥字符串（前16字节有效）

和加密模式一样，可以使用长度为64字节的混淆密钥文件替代密钥字符串进行输入，此时需要提供混淆密钥文件的文件名，例如
```
aeszero -d file_in -o file_out -f key_file
```

解密模式下如果不提供密钥或混淆密钥文件，程序会直接报错，

使用示例
```
Usage: aeszero -e|-d <infile> -k <keystring> | -f <mixkeyfile> -o <outfile>
Example:
Encrypt (1): aeszero -e <infile> -o <outfile>
Encrypt (2): aeszero -e <infile> -k <keystring> -o <outfile>
Encrypt (3): aeszero -e <infile> -f <keyfile> -o <outfile>
Decrypt (1): aeszero -d <infile> -k <keystring> -o <outfile>
Decrypt (2): aeszero -d <infile> -f <keyfile> -o <outfile>
```


为了便于操作，支持单参数的智能模式：（智能模式下不会输出任何信息到控制台）

- 如果输入的参数文件名以`.aes0`结尾，进入解密模式：记参数为`xxx.aes0`，推测密钥文件为`xxx.aes0.key`，推测输出文件为`xxx.aes0.dec`，相当于如下形式
```
aeszero -d xxx.aes0 -f xxx.aes0.key -o xxx.aes0.dec
```

- 否则进入加密模式：记参数为`xxx`，自动生成随机密钥，推测输出文件为`xxx.aes0`，相当于如下形式
```
aeszero -e xxx -o xxx.aes0
```


实现细节：

1. 输入的密钥字符串如果不足16位会生成随机的可见字符串补足16位，如果超过16位则会截断；（加密和解密均是）
2. 加密的原始文件长度不是16位的整数倍，采用PKCS7策略，填充若干字节以保证16位的整数倍，填充的字节内容就是当前填充字节数；如果恰好是16位的整数倍，则填充16个字节的16；
3. 算法实际上采用了16字节的密钥，但是在导出时采用了简单的混淆处理，加入随机字符串使得导出64字节的混淆密钥。


完整的选项支持如下，每一行的几个形式都是等价的
```
-e -E --encrypt
-d -D --decrypt
-k -K --key
-f -F --keyfile
-o -O --output
-q -Q --quiet
-v -V --version
-h -H --help
```
其中`-q`选项用于关闭控制台输出，`-v`和`-h`输出版本信息和帮助。
