echo on
protoc --descriptor_set_out=msg.protobin --include_imports msg.proto 
protogen msg.protobin  
