// bool checkInterval(unsigned long interval, unsigned long &previousMillis)
// {
//   // Mendapatkan waktu sekarang
//   unsigned long currentMillis = millis();

//   // Memeriksa apakah sudah melewati interval waktu yang ditentukan
//   if (currentMillis - previousMillis >= interval)
//   {
//     // Menyimpan waktu terakhir
//     previousMillis = currentMillis;

//     return true;
//   }

//   return false;
// }

// void pembacaanRpm (){
//     if (checkInterval(intervalRpm, milisRpm)) {
//         rpmKanan = encKananAVal * (60000 / intervalRpm) / perRotasi;
//         rpmKiri = encKiriAVal * (60000 / intervalRpm) / perRotasi;

//         encKananAVal = 0;
//         encKiriAVal = 0;
//     }
// }

// void encKanan(){
//   encKananAVal++;

//   // if ((digitalRead(encKananA) == HIGH) != (digitalRead(encKananB) == LOW))
//   // {
//   //   encKananValJarak++;
//   // }
//   // else
//   // {
//   //   encKananValJarak--;
//   // }
// }

// void encKiri(){
//   encKiriAVal++;

//   // if ((digitalRead(encKiriA) == HIGH) != (digitalRead(encKiriB) == LOW))
//   // {
//   //   encKiriValJarak++;
//   // }
//   // else
//   // {
//   //   encKiriValJarak--;
//   // }
// }

