#ifndef INTERNET_UTILITIES_HH_INCLUDED
#define INTERNET_UTILITIES_HH_INCLUDED

#include "intersock.hpp"

namespace _508
{

/** @name  calc_length
    @brief calcule la longueur du message � recevoir
    @param data  le d�but du message
    @param l_l  la longueur en nombre de caract�res de la longueur du message
**/
size_t calc_length(const char *data, uint8_t l_l);

/*  Receive a message formatted using our own method from &sock and append it to recv,
    stopping if finished is suddenly set to true.
    sock should be set to NonBlocking beforehand.

    Return:
        - sf::Socket::NotReady if was interrupted by finished
        - sf::Socket::Error or sf::Socket::Disconnected if such was the error
        - sf::Socket::Done if everything went well
*/
interface::Socket::Status recv_body(interface::TCPSocket &sock, std::string &recv, const bool &finished);

/*  Send a formmatted message stopping if finished is suddenly set to true.
    sock should be set to NonBlocking beforehand.

    Return:
        - sf::Socket::NotReady if was interrupted by finished
        - sf::Socket::Error or sf::Socket::Disconnected if such was the error
        - sf::Socket::Done if everything went well
*/
interface::Socket::Status send_body(interface::TCPSocket &sock, const std::string &send, const bool &finished);
interface::Socket::Status send_body(interface::TCPSocket &sock, const char* send, size_t length, const bool &finished);

/** @name  calc_l_l
    @brief calcule la longueur de la longueur du message � envoyer
    @param length  la longueur du message � envoyer
**/
uint8_t calc_l_l(size_t length);

/** @name  format_length
    @brief rentre la longueur du message � envoyer dans la string donn�e � partir de la position 1
    @param length la longueur du message � envoyer
    @param l_l    la longueur de la longueur du message � envoyer
    @param buffer la string dans laquelle on �crit

    suppose que la string est d�j� assez grande
**/
void format_length(size_t length, uint8_t l_l, std::string &buffer);

/* formatte la premi�re string � partir de la deuxi�me.

   Ne pas passer deux fois le m�me objet ou vous aurez de mauvais r�sultats...
*/
void format_string(std::string &formatted, const std::string &base);
void format_string(std::string &formatted, const char *base, size_t length);


}//namespace _508

#endif // INTERNET_UTILITIES_HH_INCLUDED
