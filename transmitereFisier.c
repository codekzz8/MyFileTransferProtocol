//PRIMIRE CLIENT FISIER
output = fopen("image.png", "wb");
        if (output == NULL)
        {
            perror("Eroare la deschidere fisier!\n");
            exit(1);
        }

        /* trimiterea mesajului la server */
        if (write(sd, msg, 100) <= 0)
        {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }

        /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
        size_t read_bytes, write_bytes;
        do
        {
            read_bytes = read(sd, buff, sizeof buff);
            printf("[client]read_bytes = %d\n", read_bytes);
            if (read_bytes < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            write_bytes = fwrite(buff, 1, read_bytes, output);
            bzero(buff, sizeof buff);
        } while (read_bytes > 0);
        fclose(output);
        /*
  if (read (sd, msg, 100) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  */
        /* afisam mesajul primit */
        printf("[client]Fisierul a fost primit!\n");

//TRIMITERE FISIER SERVER
char buff[8192];
        char fileName[100];
        if (read(fd, fileName, 100) < 0)
        {
          perror("[server]Eroare la citire nume fisier.\n");
          return errno;
        }

        FILE *input;
        input = fopen("index.png", "rb");
        if (input == NULL)
        {
          perror("[server]Eroare la deschidere fisier.\n");
          return errno;
        }

        size_t read_bytes, write_bytes;
        do
        {
          read_bytes = fread(buff, 1, sizeof buff, input);
          if (read_bytes < 0)
          {
            perror("[server]Eroare la citire din fisier.\n");
            return errno;
          }
          write_bytes = write(fd, buff, read_bytes);
          if (write_bytes < 0)
          {
            perror("[server]Eroare la scriere catre client.\n");
            return errno;
          }
          printf("[server]write_bytes = %d\n", write_bytes);
          bzero(buff, sizeof buff);
        } while (read_bytes > 0);
        fclose(input);