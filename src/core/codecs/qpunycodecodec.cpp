#include "qpunycodecodec_p.h"

#define MAXINT ((uint)((uint)(-1)>>1))

static const uint base = 36;
static const uint tmin = 1;
static const uint tmax = 26;
static const uint skew = 38;
static const uint damp = 700;
static const uint initial_bias = 72;
static const uint initial_n = 128;

inline uint adapt(uint delta, uint numpoints, bool firsttime)
{
    delta /= (firsttime ? damp : 2);
    delta += (delta / numpoints);

    uint k = 0;
    for (; delta > ((base - tmin) * tmax) / 2; k += base)
        delta /= (base - tmin);

    return k + (((base - tmin + 1) * delta) / (delta + skew));
}

inline char encodeDigit(uint digit)
{
  return digit + 22 + 75 * (digit < 26);
}

const char *QPunycodeCodec::name() const
{
    static const char codecName[] = "Punycode";
    return codecName;
}

int QPunycodeCodec::mibEnum() const
{
    return 2242;
}

int QPunycodeCodec::heuristicContentMatch(const char *chars, int len) const
{
    if (len < 4) return -1;
    QByteArray prefix(chars, len);
    return prefix.toLower().startsWith("xn--") ? 1 : -1;
}


QByteArray QPunycodeCodec::fromUnicode(const QString& uc, int &len_in_out) const
{
    uint n = initial_n;
    uint delta = 0;
    uint bias = initial_bias;

    // assume that the size of output will be smaller than the number
    // of input characters.
    QByteArray output;

    int ucLength = len_in_out;
    if (ucLength > uc.length() || ucLength < 0)
        ucLength = uc.length();

    // copy all basic code points verbatim to output.
    for (uint j = 0; j < (uint) ucLength; ++j) {
        ushort js = uc.at(j).unicode();
        if (js < 0x80)
            output += js;
    }

    // if there were only basic code points, just return them
    // directly; don't do any encoding.
    if (output.size() == ucLength)
        return output;

    // h and b now contain the number of basic code points in input.
    uint b = output.size();
    uint h = output.size();

    // if basic code points were copied, add the delimiter character.
    if (h > 0) output += 0x2d;

    // while there are still unprocessed non-basic code points left in
    // the input string...
    while (h < (uint) ucLength) {

        // find the character in the input string with the lowest
        // unicode value.
        uint m = MAXINT;
        uint j;
        for (j = 0; j < (uint) ucLength; ++j) {
            if (uc.at(j).unicode() >= n && uc.at(j).unicode() < m)
                m = (uint) uc.at(j).unicode();
        }

        // reject out-of-bounds unicode characters
        if (m - n > (MAXINT - delta) / (h + 1))
            return ""; // punycode_overflow

        delta += (m - n) * (h + 1);
        n = m;

        // for each code point in the input string
        for (j = 0; j < (uint) ucLength; ++j) {

            // increase delta until we reach the character with the
            // lowest unicode code. fail if delta overflows.
            if (uc.at(j).unicode() < n) {
                ++delta;
                if (!delta)
                    return ""; // punycode_overflow
            }

            // if j is the index of the character with the lowest
            // unicode code...
            if (uc.at(j).unicode() == n) {
                uint qq;
                uint k;
                uint t;

                // insert the variable length delta integer; fail on
                // overflow.
                for (qq = delta, k = base;; k += base) {
                    // stop generating digits when the threshold is
                    // detected.
                    t = (k <= bias) ? tmin : (k >= bias + tmax) ? tmax : k - bias;
                    if (qq < t) break;

                    output += encodeDigit(t + (qq - t) % (base - t));
                    qq = (qq - t) / (base - t);
                }

                output += encodeDigit(qq);
                bias = adapt(delta, h + 1, h == b);
                delta = 0;
                ++h;
            }
        }

        ++delta;
        ++n;
    }

    // prepend ACE prefix
    output.prepend("xn--");
    return output;

}

QString QPunycodeCodec::toUnicode(const char *chars, int len) const
{
    uint n = initial_n;
    uint i = 0;
    uint bias = initial_bias;

    // strip any ACE prefix
    QByteArray input(chars, len);
    QByteArray inputTrimmed = (input.startsWith("xn--") ? input.mid(4) : input);

    // find the last delimiter character '-' in the input array. copy
    // all data before this delimiter directly to the output array.
    int delimiterPos = inputTrimmed.lastIndexOf(0x2d);
    QString output = delimiterPos == -1 ? QString("") : inputTrimmed.left(delimiterPos);

    // if a delimiter was found, skip to the position after it;
    // otherwise start at the front of the input string. everything
    // before the delimiter is assumed to be basic code points.
    uint cnt = delimiterPos ? delimiterPos + 1 : 0;

    // loop through the rest of the input string, inserting non-basic
    // characters into output as we go.
    while (cnt < (uint) inputTrimmed.size()) {
        uint oldi = i;
        uint w = 1;

        // find the next index for inserting a non-basic character.
        for (uint k = base; cnt < (uint) inputTrimmed.length(); k += base) {
            // grab a character from the punycode input and find its
            // delta digit (each digit code is part of the
            // variable-length integer delta)
            uint digit = inputTrimmed.at(cnt++);
            if (digit - 48 < 10) digit -= 22;
            else if (digit - 65 < 26) digit -= 65;
            else if (digit - 97 < 26) digit -= 97;
            else digit = base;

            // reject out of range digits
            if (digit >= base || digit > (MAXINT - i) / w)
                return "";

            i += (digit * w);

            // detect threshold to stop reading delta digits
            uint t;
            if (k <= bias) t = tmin;
            else if (k >= bias + tmax) t = tmax;
            else t = k - bias;
            if (digit < t) break;

            w *= (base - t);
        }

        // find new bias and calculate the next non-basic code
        // character.
        bias = adapt(i - oldi, output.length() + 1, oldi == 0);
        n += i / (output.length() + 1);

        // allow the deltas to wrap around
        i %= (output.length() + 1);

        // insert the character n at position i
        output.insert((uint) i, QChar((ushort) n));

        ++i;
    }

    return output;
}
