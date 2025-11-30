int main() {
    int* p;
    p = 0x00000200;
    *p = 4;
    int x;
    x = *p;
    *(p + 1) = 9;
    int y;
    y = *(p + 1);

    int temp1 = x + 48;
    int temp2 = y + 48;

    output(x);
    output(y);

    while(1){
        
    }
    return 0;
}
