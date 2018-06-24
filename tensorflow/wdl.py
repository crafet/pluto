#coding=utf-8
"""ctr wide deep model"""


import tensorflow as tf
import numpy as np

from tensorflowonspark import TFNode
import sys,os



from official.utils.misc import model_helpers
from official.utils.logs import hooks_helper



default_data_path = "hdfs://appcluster-cdh/user/root/yilinliu/tfos/data/20180620/"
default_train_files = [default_data_path + "part-%05d"%i for i in range(256)]

#default_train_files = ["hdfs://appcluster-cdh/user/root/yilinliu/tfos/data/20180620/part-00000"]
default_eval_path = "hdfs://appcluster-cdh/user/root/yilinliu/tfos/data/20180621"
#default_eval_files = ["hdfs://appcluster-cdh/user/root/yilinliu/tfos/data/20180620/part-00100"]
default_eval_files = [default_eval_path + "part-%05d"%i for i in range(256)]

_NUM_EXAMPLES = {
    'train': 32561,
    'validation': 16281,
}

## tpindex data 53 columns
_CSV_COLUMNS = [
    "platformid","channel","time_stamp","averid","pctr",
    "machine","network","order_index","loc_id","loc_name",
    "loc_type","loc_plat","loc_site","loc_url","view_order",
    "advertiser1","tuwenid1","industry_dap","client_id1","is_click",
    "mixedID","gender","age","area","scene",
    "education","income","vocation","interest_num","interest_shuping",
    "creative_md51","is_download","industry_first","industry_second","clk_last7day",
    "imp_last7day","clk2_last7day","imp2_last7day","interest","loid",
    "device_id","soid","trackid","optimum_status","track_type",
    "conv_typeo","button_conv_value","hours_ago","days_ago","pcvr",
    "cver","article_id","tp_index"
]


_CSV_COLUMN_DEFAULTS = [
    ['0'],['0'],['0'],['0'],['0'],
    ['0'],['0'],['0'],['0'],['0'],
    ['0'],['0'],['0'],['0'],['0'],
    ['0'],['0'],['0'],['0'],['0'],
    ['0'],['0'],['0'],['0'],['0'],
    ['0'],['0'],['0'],['0'],['0'],
    ['0'],['0'],['0'],['0'],['0'],
    ['0'],['0'],['0'],['0'],['0'],
    ['0'],['0'],['0'],['0'],['0'],
    ['0'],['0'],['0'],['0'],['0'],
    ['0'],['0'],['0']
]

LOSS_PREFIX = {'wide': 'linear/', 'deep': 'dnn/'}

selected  = ["loc_id", "view_order", "advertiser1", "tuwenid1", "industry_dap",
             "gender", "age", "area", "channel", "platformid",
             "network"]

def input_fn(data_file, num_epochs, shuffle, batch_size):

    def select_features(features):
        d = {}
        for k in selected:
            d[k] = features[k]
        return d
    def parse_csv(value):
        print('Parsing', data_file)
        columns = tf.decode_csv(value, record_defaults=_CSV_COLUMN_DEFAULTS, field_delim="\t")
        features = dict(zip(_CSV_COLUMNS, columns))
        labels = features.pop('is_click')
        selected_features = select_features(features)
        return selected_features, tf.equal(labels, "1")

    # Extract lines from input files using the Dataset API.
    dataset = tf.data.TextLineDataset(data_file)

    if shuffle:
        dataset = dataset.shuffle(buffer_size=_NUM_EXAMPLES['train'])

    dataset = dataset.map(parse_csv, num_parallel_calls=5)

    # We call repeat after shuffling, rather than before, to prevent separate
    # epochs from blending together.
    dataset = dataset.repeat(num_epochs)
    dataset = dataset.batch(batch_size)
    return dataset

## bussiness oriented
bucket_size_dict = {
    "loc_id": list(np.linspace(1000, 5000000, 1000)),
    "advertiser1":  list(np.linspace(0, 2147483647, 1000)),
    "industry_dap": list(np.linspace(0, 2147483647, 500)),
    "gender": list(np.linspace(0, 10, 5)),
    "age": list(np.linspace(0, 150, 150)),
    "view_order": list(np.linspace(0, 2147483647, 20000)),
    "channel": list(np.linspace(0, 10000000, 100)),
    "platformid":list(np.linspace(0, 200, 100)),
    "tuwenid1": list(np.linspace(0,20000, 10)),
    "network": list(np.linspace(0,50, 10))
}

## build columns
def build_model_columns():
    def _column_name(n):
        return n + "_column"
    column_dict = {}
    for k in selected:
        column_dict[_column_name(k)] = tf.feature_column.numeric_column(key=k)
        print "column:" + _column_name(k)

    base_columns = [column_dict[_column_name(k)] for k in selected]

    def _build_bucket_column(name):
        buckets = bucket_size_dict[name]
        return tf.feature_column.bucketized_column(column_dict[_column_name(name)], buckets)

    loc_bc = _build_bucket_column("loc_id")
    adv_bc = _build_bucket_column("advertiser1")
    industry_dap_bc = _build_bucket_column("industry_dap")
    gender_bc = _build_bucket_column("gender")
    age_bc = _build_bucket_column("age")
    order_bc = _build_bucket_column("view_order")
    channel_bc = _build_bucket_column("channel")
    plat_bc = _build_bucket_column("platformid")

    tuwen_bc = _build_bucket_column("tuwenid1")
    network_bc = _build_bucket_column("network")

    crossed_columns = [
        tf.feature_column.crossed_column([loc_bc, adv_bc], hash_bucket_size=20000),
        tf.feature_column.crossed_column([loc_bc, order_bc], hash_bucket_size=4000000),
        tf.feature_column.crossed_column([loc_bc, industry_dap_bc], hash_bucket_size=50000),
        tf.feature_column.crossed_column([loc_bc, gender_bc], hash_bucket_size=2000),
        tf.feature_column.crossed_column([loc_bc, age_bc], hash_bucket_size=20000),
        tf.feature_column.crossed_column([order_bc, gender_bc], hash_bucket_size=50000),
        tf.feature_column.crossed_column([order_bc, age_bc], hash_bucket_size=1000000),
        tf.feature_column.crossed_column([industry_dap_bc, gender_bc], hash_bucket_size=1000),
        tf.feature_column.crossed_column([industry_dap_bc, age_bc], hash_bucket_size=200000),
        tf.feature_column.crossed_column([gender_bc, age_bc], hash_bucket_size=500),
        tf.feature_column.crossed_column([channel_bc, order_bc], hash_bucket_size=500000),
        tf.feature_column.crossed_column([plat_bc, order_bc], hash_bucket_size=200000),
        tf.feature_column.crossed_column([network_bc, order_bc], hash_bucket_size=200000),
        tf.feature_column.crossed_column([channel_bc, gender_bc], hash_bucket_size=500),
        tf.feature_column.crossed_column([channel_bc, age_bc], hash_bucket_size=5000),
        tf.feature_column.crossed_column([plat_bc, gender_bc], hash_bucket_size=100),
        tf.feature_column.crossed_column([plat_bc, age_bc], hash_bucket_size=1000),
        tf.feature_column.crossed_column([network_bc, gender_bc], hash_bucket_size=50),
        tf.feature_column.crossed_column([channel_bc, tuwen_bc], hash_bucket_size=500),
        tf.feature_column.crossed_column([plat_bc, tuwen_bc], hash_bucket_size=500),
        tf.feature_column.crossed_column([network_bc, tuwen_bc], hash_bucket_size=100),
        tf.feature_column.crossed_column([channel_bc, industry_dap_bc], hash_bucket_size=5000),
    ]

    wide_columns = base_columns + crossed_columns

    deep_columns = [
        column_dict[_column_name("age")],
        column_dict[_column_name("gender")],
        column_dict[_column_name("area")],
        column_dict[_column_name("network")],
        column_dict[_column_name("platformid")],
        column_dict[_column_name("advertiser1")],
    ]

    return wide_columns, deep_columns


##business oriented
feature_dimension_dict = {
    "loc_id": 500,
    "view_order": 20000,
    "advertiser1": 1000,
    "tuwenid1": 100,
    "industry_dap": 100,
    "gender": 5,
    "age": 200,
    "area": 2000,
    "channel": 100,
    "platformid": 50,
    "network": 50,
}

embedding_dimension_dict = {
    "loc_id": 3,
    "view_order": 8,
    "advertiser1": 3,
    "tuwenid1": 3,
    "industry_dap": 3,
    "gender": 3,
    "age": 3,
    "area": 8,
    "channel": 3,
    "platformid": 3,
    "network": 3,
}

def build_model_columns_hash():
    def _column_name(k):
        return k + "_column"
    def _hash_column(k):
        return tf.feature_column.categorical_column_with_hash_bucket(key=k,
                                                       hash_bucket_size=feature_dimension_dict[k])

    column_dict = {}
    ## actually some column like gender, tuwen could use tf.feature_column.categorical_column_with_identity
    ## since these features are sparse with few dimensions, here we use hash for all feature columns
    for k in selected:
        column_dict[k] = _hash_column(k)

    base_columns = [column_dict[k] for k in selected]
    crossed_columns = [
        tf.feature_column.crossed_column(keys=["loc_id", "advertiser1"], hash_bucket_size=20000),
        tf.feature_column.crossed_column(keys=["loc_id", "view_order"], hash_bucket_size=4000000),
        tf.feature_column.crossed_column(keys=["loc_id", "industry_dap"], hash_bucket_size=50000),
        tf.feature_column.crossed_column(keys=["loc_id", "gender"], hash_bucket_size=2000),
        tf.feature_column.crossed_column(keys=["loc_id", "age"], hash_bucket_size=20000),
        tf.feature_column.crossed_column(keys=["view_order", "gender"], hash_bucket_size=50000),
        tf.feature_column.crossed_column(keys=["view_order", "age"], hash_bucket_size=1000000),
        tf.feature_column.crossed_column(keys=["industry_dap", "gender"], hash_bucket_size=1000),
        tf.feature_column.crossed_column(keys=["industry_dap", "age"], hash_bucket_size=200000),

        tf.feature_column.crossed_column(keys=["gender", "age"], hash_bucket_size=500),
        tf.feature_column.crossed_column(keys=["channel", "view_order"], hash_bucket_size=500000),
        tf.feature_column.crossed_column(keys=["platformid", "view_order"], hash_bucket_size=200000),
        tf.feature_column.crossed_column(keys=["network", "view_order"], hash_bucket_size=200000),
        tf.feature_column.crossed_column(keys=["channel", "gender"], hash_bucket_size=500),
        tf.feature_column.crossed_column(keys=["channel", "age"], hash_bucket_size=5000),
        tf.feature_column.crossed_column(keys=["platformid", "gender"], hash_bucket_size=100),

        tf.feature_column.crossed_column(keys=["platformid", "age"], hash_bucket_size=1000),
        tf.feature_column.crossed_column(keys=["network", "gender"], hash_bucket_size=50),
        tf.feature_column.crossed_column(keys=["channel", "tuwenid1"], hash_bucket_size=500),

        tf.feature_column.crossed_column(keys=["platformid", "tuwenid1"], hash_bucket_size=500),
        tf.feature_column.crossed_column(keys=["network", "tuwenid1"], hash_bucket_size=100),
        tf.feature_column.crossed_column(keys=["channel", "industry_dap"], hash_bucket_size=5000),
    ]

    wide_columns = base_columns + crossed_columns
    deep_columns = [tf.feature_column.embedding_column(column_dict[k], embedding_dimension_dict[k]) for k in selected]
    return wide_columns, deep_columns


def build_estimator(model_dir, model_type):
    """Build an estimator appropriate for the given model type."""
    wide_columns, deep_columns = build_model_columns_hash()
    hidden_units = [100, 75, 50, 25]

    # Create a tf.estimator.RunConfig to ensure the model is run on CPU, which
    # trains faster than GPU for this model.
    run_config = tf.estimator.RunConfig().replace(
        session_config=tf.ConfigProto(device_count={'GPU': 0}))

    if model_type == 'wide':
        return tf.estimator.LinearClassifier(
            model_dir=model_dir,
            feature_columns=wide_columns,
            config=run_config)
    elif model_type == 'deep':
        return tf.estimator.DNNClassifier(
            model_dir=model_dir,
            feature_columns=deep_columns,
            hidden_units=hidden_units,
            config=run_config)
    else:
        return tf.estimator.DNNLinearCombinedClassifier(
            model_dir=model_dir,
            linear_feature_columns=wide_columns,
            dnn_feature_columns=deep_columns,
            dnn_hidden_units=hidden_units,
            config=run_config)

def export_model(model, model_type, export_dir):
    """Export to SavedModel format.

    Args:
      model: Estimator object
      model_type: string indicating model type. "wide", "deep" or "wide_deep"
      export_dir: directory to export the model.
    """
    wide_columns, deep_columns = build_model_columns()
    if model_type == 'wide':
        columns = wide_columns
    elif model_type == 'deep':
        columns = deep_columns
    else:
        columns = wide_columns + deep_columns
    feature_spec = tf.feature_column.make_parse_example_spec(columns)
    example_input_fn = (
        tf.estimator.export.build_parsing_serving_input_receiver_fn(feature_spec))
    model.export_savedmodel(export_dir, example_input_fn)

def main_func(args, ctx):
    model = build_estimator(args.model_dir, args.model_type)
    #train_file = os.path.join(args.data_dir, 'adult.data')
    #test_file = os.path.join(args.data_dir, 'adult.test')
    train_file = [name for idx, name in enumerate(default_train_files) if idx % args.task_num == ctx.task_index]
    #train_file = default_train_files
    #test_file = default_eval_files
    test_file = [name for idx, name in enumerate(default_eval_files) if idx % args.task_num == ctx.task_index]

    print "train_files:", train_file, "==============\n test_file:", test_file

    def train_input_fn():
        return input_fn(train_file, args.train_epochs, False, args.batch_size)

    def eval_input_fn():
        return input_fn(test_file, 1, False, args.batch_size)


    loss_prefix = LOSS_PREFIX.get(args.model_type, '')

    print("start for with: train_epochs {0}, between {1}".format(args.train_epochs, args.epochs_between_evals))

    tensors_to_log={'average_loss': loss_prefix + 'head/truediv', 'loss': loss_prefix + 'head/weighted_loss/Sum'}
    logging_hook = tf.train.LoggingTensorHook(tensors=tensors_to_log, every_n_iter=1000)
    train_spec = tf.estimator.TrainSpec(input_fn=train_input_fn, max_steps=args.max_steps,hooks=[logging_hook])
    eval_spec = tf.estimator.EvalSpec(input_fn=train_input_fn)
    tf.estimator.train_and_evaluate(model,train_spec,eval_spec)


    if args.export_dir is not None and ctx.task_index == 0:
        print("export: " + args.export_dir)
        export_model(model, args.model_type, args.export_dir)

if __name__ == '__main__':
    import argparse
    from pyspark.context import SparkContext
    from pyspark.conf import SparkConf
    from tensorflowonspark import TFCluster

    sc = SparkContext(conf=SparkConf().setAppName("dist_tfos_ctr_yilinliu"))
    executors = sc._conf.get("spark.executor.instances")
    num_executors = int(executors) if executors is not None else 1
    num_ps = 1


    parser = argparse.ArgumentParser()
    parser.add_argument("--batch_size", help="number of records per batch", type=int, default=1000)
    parser.add_argument("--cluster_size", help="number of nodes in the cluster", type=int, default=num_executors)
    parser.add_argument("--train_epochs", help="number of epochs of training data", type=int, default=2)
    parser.add_argument("--epochs_between_evals", help="number of epochs of training data", type=int, default=1)

    parser.add_argument("--export_dir", help="directory to export saved_model",default="hdfs://appcluster-cdh/user/root/yilinliu/tfos/model/20180621")
    parser.add_argument("--images", help="HDFS path to MNIST images in parallelized CSV format")
    parser.add_argument("--input_mode", help="input mode (tf|spark)", default="tf")
    parser.add_argument("--labels", help="HDFS path to MNIST labels in parallelized CSV format")
    parser.add_argument("--model_dir", help="directory to write model checkpoints",default="hdfs://appcluster-cdh/user/root/yilinliu/tfos/model/20180622")
    parser.add_argument("--data_dir", help="directory to write model checkpoints", default="hdfs://appcluster-cdh/user/root/yilinliu/tfos/input/")
    parser.add_argument("--model_type", help="directory to write model checkpoints",default="wide_deep")
    parser.add_argument("--num_ps", help="number of ps nodes", type=int, default=1)
    parser.add_argument("--task_num", help="number of worker nodes", type=int, default=1)
    parser.add_argument("--max_steps", help="max number of steps to train", type=int, default=20000)
    parser.add_argument("--tensorboard", help="launch tensorboard process", action="store_true")

    args = parser.parse_args()
    print("args:", args)

    assert(args.num_ps + args.task_num == num_executors)

    cluster = TFCluster.run(sc, main_func, args, args.cluster_size, args.num_ps, args.tensorboard, TFCluster.InputMode.TENSORFLOW,
                            log_dir=args.model_dir, master_node='master')
    cluster.shutdown()
