wide and deep model practices



Background

open source wdl model could by found here [tf models-wide_deep](https://github.com/tensorflow/models/tree/master/official/wide_deep)

however, it only could be run locally, after refine it to distribution version, I put it running on spark using tfos[TensorflowOnSpark](https://github.com/yahoo/TensorFlowOnSpark/).

Practices

tf.feature_column.*提供了一系列对特征列的处理api。

如构建categorical的api包括：

tf.feature_column.categorical_column_with_hash_bucket

tf.feature_column.categorical_column_with_identity

tf.feature_column.categorical_column_with_vocabulary_*

tf.feature_column.crossed_column()

tf.feature_column.embedding_column

categorical_column_with_ 系列构建出_CategoricalColumn类型

通过把tf.feature_column的所有特征处理接口彻底研究后，一般的处理策略是：

1. 如果是非常sparse的特征列，将特征当作tf.string来处理，（即不刻意使用numeric_column接口来指明特征为integer类型）可以考虑使用_with_hash_bucket这个api，这样的好处是在构建crossed_colulmn的时候，支持直接使用tf.string作为key进行cross，如
   tf.feature_column.crossed_column(["locid", "age")], hash_bucket_size=10000)
2. 由于crossed_column不直接支持numeric_column作为cross的输入特征，因此对于numeric_column需要通过categorical_column_with_identity或者tf.feature_column.bucketized_column()来转变成_CategoricalColumn类型再输入到crossed_column接口中才可以进行cross 。-- 这一点研究了好久
3. 对于DNN部分，一般特征需要做embedding，embedding_column支持 CategoricalColumn 类型，也支持tf.feature_colulmn.numeric_column接口产出的_NumericColumn类型，因此对于hash_with_bucket产出的特征列，可以直接embedding然后输入到DNN。当然也可以使用tf.feature_column.indicator_colulmn来进行one-hot编码，但是不如embedding来的方便，同时embedding还可以指定不同的embedding dimension，如代码中的embedding_dimension_dict [embedding_dimension_dict](https://github.com/crafet/pluto/blob/master/tensorflow/wdl.py#L191)



EoF

