# P3RP/1 — contrat du prototype LAN

P3RP/1 transporte une **unité d'accès H.264 Annex-B complète** par image. Il est prévu exclusivement pour un LAN de confiance entre le relais compagnon et la PS3.

- vidéo : UDP `9295` (PS3 en écoute) ;
- contrôle/discovery best-effort : UDP `9296` (le relais peut écouter `P3RP/1 HELLO 720P60 H264`) ;
- MTU applicative maximale : `1200` octets ;
- charge utile maximale : `1184` octets ;
- plafond d'une unité d'accès : `4 MiB` ;
- ordre : les fragments doivent arriver dans l'ordre ; une perte abandonne l'image complète.

## En-tête de 16 octets

| Offset | Taille | Champ | Valeur |
|---:|---:|---|---|
| 0 | 4 | magic | ASCII `P3RP` |
| 4 | 1 | version | `1` |
| 5 | 1 | flags | bit 0 : IDR |
| 6 | 4 | frame_id | entier non signé big-endian |
| 10 | 2 | fragment_index | entier non signé big-endian, base 0 |
| 12 | 2 | fragment_count | entier non signé big-endian |
| 14 | 2 | payload_len | entier non signé big-endian |

La PS3 valide magic/version/longueurs, refuse les fragments hors ordre, ne réassemble jamais au-delà de 4 MiB et appelle `ps3video_submit_annexb_access_unit()` uniquement quand la dernière charge utile est reçue.

## Sécurité

P3RP/1 ne contient volontairement ni token PSN, ni Account ID, ni clé d'enregistrement, ni clé de session Remote Play. Cette première version ne chiffre pas les paquets et ne signe pas les sources : ne l'exposer ni à Internet, ni à un VLAN invité, ni à un réseau Wi-Fi non fiable. Une phase suivante devra ajouter authentification mutuelle et réordonnancement/FEC avant tout emploi plus large.
