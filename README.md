# univr-progetto-so-2023
Progetto corso Sistemi Operativi anno 2022/2023

## Utilizzo
- Compilare con make `$ make`

### **Server**
- Lanciare il server con il comando: <br>
  `./bin/F4Server {righe} {colonne} {player1Symbol} {player2Symbol}`

- Leggere il `messaqueueId` in output e passarlo come parametro ad `F4Client`


### **Client**
 - Lanciare il client con il comando `./bin/F4Client {connectionQueueId} {nomeGiocatore}`

- Giocare!