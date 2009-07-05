#include "utilities.hh"
#include "exception.hpp"

using namespace std;

smart_ptr<string> find_line(const char *file_path, int count, bool binary)
{
    ifstream file;
    if (binary)
        file.open(file_path, ios::binary);
    else
        file.open(file_path);

    if (!file)
        throw MF_Exception(string("Error while opening file ") + file_path);

    return find_line(file, count);
}

smart_ptr<string> find_line(ifstream &file, int count)
{
    for ( ; count > 1; count-- )
    {
        file.ignore( numeric_limits<int>::max(), '\n' );
    }

    smart_ptr<string> result = new string();

    getline(file, *result);

    return result;
}

smart_ptr< fast_array<char> > get_file_content(const char *path, bool binary)
{
    std::ifstream f;
    if (binary)
    {
        f.open(path, std::ios::binary);
    } else
    {
        f.open(path);
    }

    if (!f)
    {
        fast_array<char> *ptr = new fast_array<char>(1);
        ptr[0] = 0;
        return ptr;
    }

    f.seekg(0, std::ios::end);

    size_t size = f.tellg();

    f.seekg(0, std::ios::beg);

    smart_ptr< fast_array<char> > r = new fast_array<char>(size+1);

    f.read(*r, size);
    r[0][size] = 0;

    return r;
}


inline void BitsSerializer::push_ch(char item)
{
    /* we assume that we're not on a right offset, otherwise push_cc would have optimized it */
    for (int i = 7; i >= 0; i--)
    {
        push_bit(item & (1 << i));
    }
}

void BitsSerializer::push_cc(const char *item, size_t len)
{
    bits->resize(get_clen() + len);

    /* Si jamais le compte tombe juste, c'est miraculeux */
    if (count%8==0)
    {
        memcpy(&(*bits)[count/8], item, len);
        count += len*8;

        return;
    }

    /* l� c'est la partie chiante */
    while (len > 0)
    {
        push_ch(*item);
        len--;
        item++;
    }
}

void BitsSerializer::push_l(long item, unsigned char bits_len)
{
    bits->resize((count+bits_len)/8+((count+bits_len)%8!=0));
    if (count % 8 == 0 && bits_len % 8 == 0)
    {
        for (bits_len = bits_len - 8; bits_len > 0; bits_len -= 8)
        {
            (*bits)[count/8] = item &  (0xFF << bits_len);
            count += 8;
        }
        (*bits)[count/8] = item;
        count += 8;

        return;
    }

    for (bits_len = bits_len - 1; bits_len > 0; bits_len --)
    {
        push_bit(item & (1 << bits_len));
    }
    push_bit(item & (1 << bits_len));
}

bool BitsDeserializer::pop_c(char *item, size_t char_len)
{
    if (!check_bitlen(char_len * 8))
        return false;

    if (count == 0)
    {
        memcpy(item, bits, char_len);
        bits += char_len;
    } else {
        while (char_len > 0)
        {
            *item = pop_ch();
            item ++;
            char_len --;
        }
    }

    return true;
}

void MegaSerializer::mega_push(const char * scheme, ...)
{
    va_list args;
    va_start(args, scheme);

    while( *scheme != 0 )
    {
        uint8_t n = scheme[1]-'0';
        if (*scheme == '#')
        {
            long item = (long)va_arg(args, long);
            push_l(item, n);
        } else
        {
            if (*scheme == '$')
            {
                push_str(*((const string*)va_arg(args, const string*)), n);
            } else //scheme == %
            {
                push_cc((const char*)va_arg(args, const char*), (size_t)va_arg(args, size_t), n);
            }
        }
        scheme += 2;
    }

    va_end(args);
}

bool MegaDeserializer::mega_pop(const char *scheme, ...)
{
    va_list args;
    va_start(args, scheme);

    while( *scheme != 0 )
    {
        uint8_t n = scheme[1]-'0';
        if (*scheme == '#')
        {
            void *ptr = va_arg(args, void*);
            long val;
            if (!pop_l(val, n))
                goto error;
            if (n <= 8)
            {
                *((char*)ptr) =  val;
            } else if (n <= 16)
            {
                *((uint16_t*)ptr) = val;
            } else if (n <= 32)
            {
                *((uint32_t*)ptr) = val;
            } else
            {
                *((uint64_t*)ptr) = val;
            }
        } else if (*scheme == '$')
        {
            if (!pop_str(*((string*)va_arg(args, string*)), n))
                goto error;
        } else if (*scheme == '%')
        {
            if (pop_c((char*)va_arg(args, char*), n))
                goto error;
        } else
        {
            exit(1003);
        }
        scheme += 2;
    }

    va_end(args);
    return true;

error:
    va_end(args);

    cout << "Erreur lors de la d�s�rialisation des donn�es" << endl;
    return false;
}

FileArchiver::FileArchiver(const char *path) :out(path, ios::binary), entetes(), cur_pos(0)
{
    if (!out)
    {
        throw MF_Exception("Erreur lors du chargment dans FileArchiver: " +string (path));
    }
}

void FileArchiver::add_file(const char *path)
{
    size_t long size;

    ifstream in(path, ios::binary);

    if (!in)
    {
        throw MF_Exception("FileArchiver : Erreur lors de l'addition d'un fichier: " + string(path));
    }

    // get pointer to associated buffer object
    filebuf *pbuf=in.rdbuf();

    // get file size using buffer's members
    size=pbuf->pubseekoff (0,ios::end,ios::in);
    pbuf->pubseekpos (0,ios::in);

    char buffer[size];
    pbuf->sgetn (buffer,size);

    out.write(buffer, size);

    entetes.push_back(entete(path, cur_pos, size));
    cur_pos += size;
}

void FileArchiver::finish()
{
    list<entete>::iterator it;

    for (it = entetes.begin(); it != entetes.end(); ++it)
    {
        out << it->nom << " " << it->pos << " " << it->len << endl;
    }

    out << ((entetes.back()).pos + (entetes.back()).len) << flush;
}

smart_ptr< fast_array<char> > OpenFile(const char *archive, const char *path)
{
    ifstream arc(archive, ios::binary);
    string line;
    size_t path_len;
    long remaining_len;
    size_t file_pos;
    smart_ptr< fast_array<char> > ret = new fast_array<char> ();
    char *item;
    size_t file_length;
    const char * errMess = NULL;

    if (!arc)
    {
        errMess = "Impossible d'ouvrir l'archive!";
        goto err;
    }
    /* seek last line */
    arc.seekg(0, ios::end);

    size_t size = arc.tellg();
    if (size < 21)
    {
        errMess = "Taille de l'archive trop petite!";
        goto err;
    }

    arc.seekg(-20, ios::cur);

    while (getline(arc, line))
    {;}

    size_t pos_ent = ConvertTo<size_t>(line);

    if (pos_ent >= size)
    {
        errMess = "Dans l'archive: indication aberrante du d�but des entetes fichier, qui va en dehors de l'archive!";
        goto err;
    }

    arc.clear();
    arc.seekg(pos_ent, ios::beg);

    //Voil�
    //Maintenant on recherche le fichier correspondant
    path_len = strlen(path);

    while (getline(arc, line))
    {
        if (line.length() > path_len && strncmp(line.data(), path, path_len) == 0)
        {
            break;
        }
    }

    if (arc.fail() || arc.eof())
    {
        errMess = "Impossible de trouver le fichier dans l'archive!";
        goto err;
    }

    //On a maintenant dans la string tout ce qu'il nous faut
    remaining_len = (signed)line.length() - path_len - 1;
    if (remaining_len < 3)
    {
        goto err;
    }

    item = const_cast<char*>(line.c_str()) + path_len + 1;

    file_pos = strtol(item, &item, 10);

    if (item == NULL)
    {
        errMess = "Format de l'entete du fichier invalide!";
        goto err;
    }

    file_length = ConvertTo<size_t>(item);

    if (file_pos+file_length > size)
    {
        errMess = "Erreur dans l'entete du fichier, qui dit que le fichier va plus loin que la fin de l'archive!";
        goto err;
    }

    arc.seekg(file_pos);
    //Finalement on stocke tout dans une string

    ret->resize(file_length);

    arc.read(*ret, file_length);

    return ret;

err:
    // Gestion d'erreur avanc�e lol
    throw MF_Exception(string("Erreur dans Openfile! Fichier Demand�: ")+path+" Dans: "+archive+"\nErreur: "+errMess);
}

//template <class T>
//void fast_array<T>:: resize(size_t new_size)
//{
//    T *ptr = (T*) realloc(data, new_size * sizeof(T));
//    if (ptr)
//    {
//        data = ptr;
//        size = new_size;
//    } else
//    {
//        cout << (int)ptr << "- -" << (int)data << endl;
//        exit(1202);
//    }
//}
