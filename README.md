# Proiect-Sisteme-de-Operare-SO
&emsp; Proiect Sisteme de Operare (SO) Anul 2, Semestrul 1, Facultatea de Matematica si Informatica, Universitatea din Bucuresti <br/>

### &emsp; Membrii Echipa: <br/>
Capatina Razvan Nicolae ($252$) <br/> 
Buca Mihnea Vicentiu ($252$) <br/>
Mihalache Sebastian Stefan ($252$) <br/>
Luculescu Teodor ($251$) <br/>

# DiskAnalyzer

# Setup

```
bash setup.sh
```

# Descriere

Creati un daemon care analizeaza spatiul utilizat pe un dispozitiv de stocare incepand de la o cale data, si construiti un program utilitar care permite folosirea acestei functionalitati din linia de comanda. <br>
Daemonul trebuie sa analizeze spatiul ocupat recursiv, pentru fiecare director continut, indiferent de adancime.

Utilitarul la linia de comanda se va numi "da" si trebuie sa expuna urmatoarele functionalitati: 
- [x] Crearea unui job de analiza, pornind de la un director parinte si o prioritate data
    - [x] prioritatile pot fi de la $1$ la $3$ si indica ordinea analizei in raport cu celelate joburi ($1$-low, $2$-normal, $3$-high)
    - [x] un job de analiza pentru un director care este deja parte dintr-un job de analiza, nu trebuie sa creeze task-uri suplimentare
- [x] Anularea / stergerea unui job de analiza
- [x] Intreruperea si restartarea (pause/resume) unui job de analiza
- [x] Interogarea starii unui job de analiza (preparing, in progress, done)

```
   Usage: da [OPTION]... [DIR]...
   Analyze the space occupied by the directory at [DIR]
      -a, --add                analyze a new directory path for disk usage
      -p, --priority <1/2/3>   set priority for the new analysis (works only with -a argument)
      -S, --suspend <id>       suspend task with <id>
      -R, --resume <id>        resume task with <id>
      -r, --remove <id>        remove the analysis with the given <id>
      -i, --info <id>          print status about the analysis with <id> (pending, progress, done)
      -l, --list               list all analysis tasks, with their ID and the corresponding root path
      -p, --print <id>         print the analysis report for the task with <id> if it is "done"
      -t, --terminate          terminates the daemon
```
