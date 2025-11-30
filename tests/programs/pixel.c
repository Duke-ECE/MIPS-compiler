int main()
{
    int x0;
    int y0;
    int* fb = 0x00100000;
    int width = 680;

    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            *(fb + (y0 + y) * width + (x0 + x)) = 1;
        }
    }


}