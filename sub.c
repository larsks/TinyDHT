        for (i=0; i<5; i++) {
            for (j=0; j<8; j++) {
                // signal goes low for ~ 50us before data bit
                counter = 0;
                while (!(DHTPINREG & (1<<dht->pin))) {
                    if (counter++ > 60) goto failed;
                    delayMicroseconds(1);
                }

                if (counter < 20) goto failed;

                // timing of high signal represents bit value
                counter = 0;
                while ((DHTPINREG & (1<<dht->pin))) {
                    delayMicroseconds(1);
                    if (counter++ > 100) break;
                }

                dht->data[i] <<= 1;
                dht->data[i] |= (counter > 60) ? 1 : 0;
            }
        }
