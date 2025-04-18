source ~/anaconda3/etc/profile.d/conda.sh
conda activate surrogate

cd python
python -m grpc_tools.protoc -I../protos --python_out=./proto_generated --pyi_out=./proto_generated --grpc_python_out=./proto_generated ../protos/*.proto
