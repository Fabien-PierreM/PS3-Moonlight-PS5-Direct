# Build reproductible et validation

## Prérequis

- Windows 11 avec WSL `Ubuntu-24.04` (déjà disponible sur cette machine) ;
- `git`, `make`, `curl`, `tar` dans WSL ;
- une PS3 sous CFW/HEN uniquement pour l'installation matérielle ;
- aucun SDK PS3 global n'est requis : `make prepare` télécharge un seul SDK Linux épinglé par le `Makefile`.

> Le SDK est téléchargé une seule fois. Ne lancez pas plusieurs compilations `make prepare` simultanément.

## Construire le PKG PS3

Depuis WSL, copiez d'abord l'arbre vers le système de fichiers Linux. Sur cette
machine, CMake ne peut pas écrire ses fichiers de configuration dans le montage
NTFS `/mnt/c`, alors que le build dans `/home/ubuntu` fonctionne :

```bash
mkdir -p ~/src
rm -rf ~/src/PS3-Moonlight-PS5-Relay
cp -a /mnt/c/Users/fabie/PS3-Moonlight-PS5-Relay ~/src/
cd ~/src/PS3-Moonlight-PS5-Relay
make prepare
make pkg
find build -maxdepth 1 -name 'ps3-remoteplay-relay-*.pkg' -print
```

Le résultat est un paquet NPDRM dans `build/ps3-remoteplay-relay-<YYYYMMDD_HHMMSS>.pkg`. Le recopier ensuite dans Windows, par exemple vers `C:/Users/fabie/PS3-Moonlight-PS5-Relay/dist/`. L'identité PS3 est `PS5RL0001`, donc elle n'écrase pas l'installation Moonlight `MNLT00001` d'origine.

## Vérifier la donnée P3RP/1 sans PS5

Le test n'est pas un émulateur de PS5 : il confirme seulement la forme du transport H.264 vers la PS3.

```bash
python3 -m py_compile tools/send_annexb_au.py
python3 tools/send_annexb_au.py <IPV4_DE_LA_PS3> <une-unite-d-acces-annexb.h264> --idr
```

Démarrer d'abord l'application PS3, définir l'IP du poste qui lance le test puis choisir **CONNECT TO RELAY**. Le programme PS3 écoute UDP/9295. Une unité d'accès incomplète, trop grosse ou fragmentée dans le désordre est rejetée par conception.

## Construire le composant compagnon

Le répertoire `relay/` compile une bibliothèque de paquetisation indépendamment de Chiaki-ng :

```bash
cmake -S /mnt/c/Users/fabie/PS3-Moonlight-PS5-Relay/relay \
  -B ~/builds/ps3-remoteplay-relay-packetizer
cmake --build ~/builds/ps3-remoteplay-relay-packetizer
ctest --test-dir ~/builds/ps3-remoteplay-relay-packetizer --output-on-failure
```

Pour compiler l'adaptateur Chiaki-ng, fournir un arbre Chiaki-ng construit séparément et sa bibliothèque :

```bash
cmake -S /mnt/c/Users/fabie/PS3-Moonlight-PS5-Relay/relay \
  -B ~/builds/ps3-remoteplay-relay-chiaki \
  -DCHIAKI_INCLUDE_DIR=/chemin/vers/chiaki-ng/lib/include \
  -DCHIAKI_LIBRARY=/chemin/vers/libchiaki.a
cmake --build ~/builds/ps3-remoteplay-relay-chiaki
```

L'intégrateur du compagnon doit créer une session Chiaki-ng autorisée avec les mécanismes officiels, appeler `chiaki_ps3_relay_set_720p_profile()`, puis installer `chiaki_ps3_relay_video_sample` avec `chiaki_session_set_video_sample_cb()`. Les secrets ne doivent pas être copiés vers la PS3, le PKG, ni un dépôt Git.

## Matrice de validation finale à faire sur matériel

| Test | Critère |
|---|---|
| Installation | Le PKG apparaît sous **Game** avec le titre PS3 Remote Play Relay. |
| Décodage | Une unité H.264 Annex-B 1280×720p60 valide est affichée sans crash. |
| Robustesse | Fragment manquant / hors ordre : image ignorée, application reste utilisable. |
| Isolation | Aucun Account ID, token, clé d'enregistrement ou secret dans `/dev_hdd0/game/PS5RL0001/USRDIR/config.ini`. |
| Interop relais | La session Chiaki-ng autorisée force H.264/720p60 et émet P3RP/1. |

Le build est reproductible ; l'interop complète PS5 reste à vérifier sur la console et le firmware réels.
