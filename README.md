# Mini Project - 3: Digital Filters

**課程**: 多媒體訊號處理  

## 1. 目的

1. 如何設計需要的 digital filter
2. 了解 digital filter 如何在 time domain 實現

## 2. 說明

給定一個琴聲譜、取樣率為48kHz、bit depth為16bits的音樂([連結](https://github.com/cychiang-ntpu/ntpu-ce-mmsp-2021/blob/master/Chapter-4/Ascience-Fast-Piano-Add-Tones.wav))，請撰寫一個 C 語言程式，將此音樂通過濾波器進行濾波，數學如下：

![equation1](https://latex.codecogs.com/svg.latex?y_L%5Bn%5D%20%3D%20x_L%5Bn%5D%20*%20h_L%5Bn%5D) (1)

![equation2](https://latex.codecogs.com/svg.latex?y_R%5Bn%5D%20%3D%20x_R%5Bn%5D%20*%20h_R%5Bn%5D) (2)

其中 x<sub>L</sub>[n] 以及 x<sub>R</sub>[n] 為原本音樂的左聲道以及右聲道音訊，h<sub>L</sub>[n] 是 band-pass filter 的 impulse response，我們希望它近似於以下的 analog filter：

![equation3](https://latex.codecogs.com/svg.latex?H_L%28j%5COmega%29%20%3D%20%5Cbegin%7Bcases%7D%202%2C%20%26%20%5Ctext%7Bif%20%7D2%5Cpi%20%5Ctimes%201000%20%3C%20%7C%5COmega%7C%20%3C%202%5Cpi%20%5Ctimes%204000%5C%5C%200%2C%20%26%20%5Ctext%7Botherwise%7D%20%5Cend%7Bcases%7D) (3)

而 h<sub>R</sub>[n] 是 band-stop filter 的 impulse response，我們希望它近似於以下的 analog filter：

![equation4](https://latex.codecogs.com/svg.latex?H_R%28j%5COmega%29%20%3D%20%5Cbegin%7Bcases%7D%200%2C%20%26%20%5Ctext%7Bif%20%7D2%5Cpi%20%5Ctimes%201000%20%3C%20%7C%5COmega%7C%20%3C%202%5Cpi%20%5Ctimes%204000%5C%5C%201%2C%20%26%20%5Ctext%7Botherwise%7D%20%5Cend%7Bcases%7D) (4)

h<sub>L</sub>[n] 以及 h<sub>R</sub>[n] 都是長度為 2M + 1 的 causal FIR (finite impulse response)，可由 window method 求出。

## 3. 規格

### 3.1. 原始碼
fir_xxxxxx.c，其中 xxxxxxxx 代表你的學號

### 3.2. 程式使用方法
```bash
fir_xxxxxx M hL.txt hR.txt YL.txt YR.txt input.wav output.wav
```

- 其中 M 就是和 impulse response 相關的參數，FIR 的 impulse response 長度是 2M + 1
- hL.txt 是一個使用 ascii 儲存 h<sub>L</sub>[n] 的檔案，使用科學記號儲存 (%)，以最高精確度表示
- hR.txt 是一個使用 ascii 儲存 h<sub>R</sub>[n] 的檔案，使用科學記號儲存 (%)，以最高精確度表示
- YL.txt 是一個使用 ascii 儲存左聲道 30 秒到 30.025 秒之間的 1200 個 sample 點之 log spectrum，使用科學記號儲存 (%)，以最高精確度表示
- YR.txt 是一個使用 ascii 儲存右聲道 30 秒到 30.025 秒之間的 1200 個 sample 點之 log spectrum，使用科學記號儲存 (%)，以最高精確度表示
- input.wav 是輸入的 48kHz、bit depth為16bits的音樂 wave file
- output.wav 是經過 band-pass 和 band-stop 的 48kHz、bit depth為16bits的音樂 wave file

### 3.3. 參數
請測試三種不同的 M：

- M = 8
- M = 32
- M = 1024