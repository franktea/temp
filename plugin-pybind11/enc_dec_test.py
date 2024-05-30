import enc_dec_wrapper

class CustomEncDec(enc_dec_wrapper.EncDec):
    def __init__(self):
        super().__init__()

    def Encrypt(self, str):
        encrypted_str = ""
        for c in str:
            encrypted_str += chr((ord(c) + 3) % 256)
        return encrypted_str

    def Decrypt(self, str):
        decrypted_str = ""
        for c in str:
            decrypted_str += chr((ord(c) - 3 + 256) % 256)
        return decrypted_str

if __name__ == "__main__":
    text = "Hello, World!"

    custom_enc_dec = CustomEncDec()
    encrypted_text = custom_enc_dec.Encrypt(text)
    print("Encrypted text:", encrypted_text)

    decrypted_text = custom_enc_dec.Decrypt(encrypted_text)
    print("Decrypted text:", decrypted_text)
