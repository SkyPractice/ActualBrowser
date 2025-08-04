# TypeWrapper Guide
# ------------------------------------------------------------
# Rules : 

## 1- A Type Wrapper Object Should Have A Member Called primitive ( the underlying primitive )

## 2- A Type Wrapper Object Should Have 2 Main Compiled Methods get, set ( getters and setters 
## for the underlying primitive type );

## 3- In The C++ Code For Every Wrapper There Should Be A Function Called genObject
## That Will Generate A Wrapper From A Primitive

## 4- In The C++ Code For Every Wrapper There Should Be A Function Called initializeWrapper,
## Which Will Initialize The Native Functions

## 4- An Type Wrapper Object Should Have A Member Called is_primitive
## That Member Should Have The Name Of The Primitive Type Like array, object, etc

### Wrappers Are Gonna Be a Bit Code Intensive But Its Worth It
### Good Luck !