int array[3000][500];
int main() {
	for (int i = 0; i < 500; i++) {
		for (int j = 0; j < 3000; j++) {
			array[j][i] = j + i;
		}
	}
}
