Copyright (c) 2018 CRI Middleware Co., Ltd.

# Version
1.00

# このファイルについて
ADX2 LE の UnrealEngine4 向けサンプルプロジェクト AtomBasicFeaturesLE について説明します。

# AtomBasicFeaturesLE プロジェクトについて
UnrealEngine4 上における ADX2 LE の使用例を示すサンプルプロジェクトです。本プロジェクトは UnrealEngine4.18.1 で作成しました。

## 使い方
AtomBasicFeatures.uproject を UnrealEngine4 (以下UE4) で開くと、CRIWAREプラグインのビルドを要求されます。ビルド処理の実行を承諾し、完了を待って下さい。ビルドが終わると UE4 Editor が開きます。Content 直下に配置されている Sample.umap を開き、Editor 上でプレビューを実行してください。FirstPerson視点のサンプルゲームが起動し、ヘリコプターが飛ぶ音とBGMが聞こえ始めたら正常な起動が完了しています。

## 機能
本サンプル内では、ADX2LE の複雑な機能までは紹介しません。以下のようなごくシンプルな音声再生の例を示しています。

- AtomComponentを使用したキューの単純再生
- 3Dポジショニング設定が適用されたキューの再生

### AtomComponentを使用したキューの単純再生
Sample.umap 上に配置されている floor と cylinder を Editor 上で確認してください。それぞれ Atom というコンポーネントが付与されています。これらは UE4 上でシンプルに ADX2 LE のサウンド機能を使うためのコンポーネントです。このコンポーネントに目的のサウンドデータ(ADX2 LEでは主に「キュー」と呼びます)を指定することで、簡単に再生することが可能です。サウンドデータのファイル自体はContent/Sound ディレクトリと Content/CriWare ディレクトリ以下に配置されています。それぞれのファイル形式の意味については、ADX2 LE SDK パッケージに同梱されている CRI_ADX2LE_UE4_Plugin_Manual_j.chm を参照してください。

### 3Dポジショニング設定が適用されたキューの再生
ADX2 LE では、キューに「3Dポジショニング」を設定することができます。3D空間上の、リスナーの位置に応じて3Dソース(音源)からの発する音の聞こえ方を自動調整する機能です。この機能について、UE4 プロジェクトやプログラム側から細かくコントロールしたり何か複雑な設定をする必要は、基本的にはありません。詳細な設定情報は全て ADX2 LE の専用データ内に埋め込まれていますし、ADX2 LE ライブラリ(CRIWARE ライブラリ)はそのデータを読み取って、自動的に音量をコントロールします。

### その他の機能
AtomComponent 以外にも多くの機能が ADX2 LE には搭載されています。詳細な使い方を知りたい場合は、ADX2 LE SDK パッケージに同梱されている CRI_ADX2LE_UE4_Plugin_Manual_j.chm を参照してください。また ADX2 LE の専用データを自分で作成したい場合は AtomCraft という専用ツールを使う必要があります。このツールのマニュアル ADX2 LE SDK に同梱されています。別途参照してください。

## 注意
### プラグインの導入手順
本サンプルプロジェクトは、CRIWARE プラグインの導入手順(インストール方法)を示すものではありません。この手順については CRI_ADX2LE_UE4_Plugin_Manual_j.chm を参照してください

### 本サンプル内の ADX2 LE データの AtomCraft プロジェクトについて
ADX2 LE SDK パッケージ内の、以下の場所に AtomCraft プロジェクトファイルを配置しています。
```
cri/pc/samples/criatom/AtomCraftWork/AtomCraftWork_samples.atmcproject
```
これを専用ツール AtomCraft で開いて内容を確認してください。

