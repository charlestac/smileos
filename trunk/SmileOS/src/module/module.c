/*********************************************************************************************************
**
** Copyright (c) 2011 - 2012  Jiao JinXing <JiaoJinXing1987@gmail.com>
**
** Licensed under the Academic Free License version 2.1
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**--------------------------------------------------------------------------------------------------------
** File name:               module.c
** Last modified Date:      2012-7-18
** Last Version:            1.0.0
** Descriptions:            �ں�ģ��
**
**--------------------------------------------------------------------------------------------------------
** Created by:              JiaoJinXing
** Created date:            2012-7-18
** Version:                 1.0.0
** Descriptions:            �����ļ�
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
** ELF �ļ���ʽ��鿴:
**
** http://os.pku.edu.cn:8080/gaikuang/submission/TN05.ELF.Format.Summary.pdf
*********************************************************************************************************/
#include "kern/kern.h"
#include "kern/mmu.h"
#include "kern/ipc.h"
#include "vfs/utils.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "module/elf.h"
#include "module/symbol_tool.h"
#include "module/module.h"
/*********************************************************************************************************
** ȫ�ֱ���
*********************************************************************************************************/
static module_t    *mod_list;                                           /*  ģ������                    */

static mutex_t      mod_mgr_lock;                                       /*  ģ�������                  */
/*********************************************************************************************************
** Function name:           module_init
** Descriptions:            ��ʼ���ں�ģ����ϵͳ
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
int module_init(void)
{
    extern int symbol_init(void);
    symbol_init();                                                      /*  ��ʼ�����ű�                */

    mod_list = NULL;                                                    /*  ��ʼ��ģ������              */

    return mutex_new(&mod_mgr_lock);                                    /*  ����ģ�������              */
}
/*********************************************************************************************************
** Function name:           module_install
** Descriptions:            ��װģ��
** input parameters:        mod                 ģ��
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
static int module_install(module_t *mod)
{
    mutex_lock(&mod_mgr_lock, 0);

    mod->next = mod_list;
    mod_list  = mod;

    mutex_unlock(&mod_mgr_lock);

    return 0;
}
/*********************************************************************************************************
** Function name:           module_remove
** Descriptions:            ��ģ������ɾ��ģ��
** input parameters:        mod                 ģ��
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
static int module_remove(module_t *mod)
{
    module_t *tmp, *prev;
    int ret = -1;

    mutex_lock(&mod_mgr_lock, 0);

    prev = NULL;
    tmp  = mod_list;
    while (tmp != NULL) {
        if (mod == tmp) {
            if (prev != NULL) {
                prev->next = tmp->next;
            } else {
                mod_list   = tmp->next;
            }
            ret = 0;
            break;
        }
        prev = tmp;
        tmp  = tmp->next;
    }

    mutex_unlock(&mod_mgr_lock);

    return ret;
}
/*********************************************************************************************************
** Function name:           module_lookup_by_key
** Descriptions:            ����ģ��
** input parameters:        key                 ģ���� KEY
** output parameters:       NONE
** Returned value:          ģ�� OR NULL
*********************************************************************************************************/
static module_t *module_lookup_by_key(uint32_t key)
{
    module_t *mod;

    mutex_lock(&mod_mgr_lock, 0);

    mod = mod_list;
    while (mod != NULL) {
        if (key == mod->key) {
            break;
        }
        mod = mod->next;
    }

    mutex_unlock(&mod_mgr_lock);

    return mod;
}
/*********************************************************************************************************
** Function name:           module_lookup
** Descriptions:            ����ģ��
** input parameters:        name                ģ����
** output parameters:       NONE
** Returned value:          ģ�� OR NULL
*********************************************************************************************************/
module_t *module_lookup(const char *name)
{
    module_t *mod;
    uint32_t key;

    if (name == NULL) {
        return NULL;
    }

    key = BKDRHash(name);

    return module_lookup_by_key(key);
}
/*********************************************************************************************************
** Function name:           module_ref_by_addr
** Descriptions:            ͨ��ģ���һ����ַ����ģ��
** input parameters:        addr                ��ַ
** output parameters:       NONE
** Returned value:          ģ�� OR NULL
*********************************************************************************************************/
module_t *module_ref_by_addr(void *addr)
{
    module_t *mod;

    mutex_lock(&mod_mgr_lock, 0);

    mod = mod_list;
    while (mod != NULL) {
        if (((char *)addr >= (char *)mod + sizeof(module_t)) &&
            ((char *)addr <  (char *)mod + sizeof(module_t) + mod->size)) {
            atomic_inc(&mod->ref);
            break;
        }
        mod = mod->next;
    }

    mutex_unlock(&mod_mgr_lock);

    return mod;
}
/*********************************************************************************************************
** Function name:           module_unref
** Descriptions:            ��ȡ��ģ�������
** input parameters:        mod                 ģ��
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
int module_unref(module_t *mod)
{
    if (mod != NULL) {
        atomic_dec(&mod->ref);
        return 0;
    } else {
        return -1;
    }
}
/*********************************************************************************************************
** Function name:           module_alloc
** Descriptions:            ����ģ��
** input parameters:        path                �ļ�·��
**                          size                �ļ���С
** output parameters:       NONE
** Returned value:          ģ�� OR NULL
*********************************************************************************************************/
static module_t *module_alloc(const char *path, size_t size)
{
    module_t *mod;

    mod = kmalloc(sizeof(module_t) + size, GFP_KERNEL);                 /*  ����ģ�鼰�仺��            */
    if (mod != NULL) {
        /*
         * ��ʼ��ģ��
         */
        strlcpy(mod->name, path, sizeof(mod->name));
        mod->key        = BKDRHash(mod->name);
        mod->elf        = (uint8_t *)mod + sizeof(module_t);
        mod->size       = size;
        mod->bss_idx    = SHN_UNDEF;
        mod->text_idx   = SHN_UNDEF;
        mod->shdrs      = NULL;
        atomic_set(&mod->ref, 0);
    }

    return mod;
}
/*********************************************************************************************************
** Function name:           module_free
** Descriptions:            ����ģ��
** input parameters:        mod                 ģ��
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
static int module_free(module_t *mod)
{
    if (mod->bss_idx != SHN_UNDEF) {                                    /*  �ͷŵ� .bss ����            */
        kfree((void *)mod->shdrs[mod->bss_idx]->sh_offset);
    }

    if (mod->shdrs != NULL) {                                           /*  �ͷŽ����ײ�����            */
        kfree(mod->shdrs);
    }

    kfree(mod);

    return 0;
}
/*********************************************************************************************************
** Function name:           arm_reloc_rel
** Descriptions:            ��ָ���� rel ��Ŀ�����ض�λ
** input parameters:        rel                 REL ��Ŀ
**                          addr                ��ַ
**                          target              Ŀ�����
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
static int arm_reloc_rel(Elf32_Rel *rel, Elf32_Addr addr, uint8_t *target)
{
    Elf32_Addr *where, tmp;
    Elf32_Sword addend;

    where = (Elf32_Addr *)(target + rel->r_offset);

    switch (ELF32_R_TYPE(rel->r_info)) {
    case R_ARM_NONE:
        break;

    case R_ARM_ABS32:
        *where += addr;
        break;

    case R_ARM_PC24:
    case R_ARM_PLT32:
    case R_ARM_CALL:
    case R_ARM_JUMP24:
        addend = *where & 0x00ffffff;
        if (addend & 0x00800000) {
            addend |= 0xff000000;
        }
        tmp    = addr - (Elf32_Addr)where + (addend << 2);
        tmp  >>= 2;
        *where = (*where & 0xff000000) | (tmp & 0x00ffffff);
        break;

    default:
        return -1;
    }
    return 0;
}
/*********************************************************************************************************
** Function name:           module_probe
** Descriptions:            ̽��ģ��
** input parameters:        mod                 ģ��
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
static int module_probe(module_t *mod)
{
    Elf32_Ehdr *ehdr;

    ehdr = (Elf32_Ehdr *)mod->elf;                                      /*  ELF �ײ�                    */

    if ((ehdr->e_ident[EI_MAG0] != ELFMAG0) ||                          /*  ħ�����                    */
        (ehdr->e_ident[EI_MAG1] != ELFMAG1) ||
        (ehdr->e_ident[EI_MAG2] != ELFMAG2) ||
        (ehdr->e_ident[EI_MAG3] != ELFMAG3)) {
        printk("ehdr->e_ident[EI_MAG0] != ELFMAG0\n");
        return -1;
    }

    if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {                        /*  �ļ�����: ������ 32 λĿ��  */
        printk("ehdr->e_ident[EI_CLASS] != ELFCLASS32\n");
        return -1;
    }

    if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {                        /*  ���ݱ��뷽ʽ: ������С��    */
        printk("ehdr->e_ident[EI_DATA] != ELFDATA2LSB\n");
        return -1;
    }

    if (ehdr->e_ident[EI_VERSION] != EV_CURRENT) {                      /*  ELF �ײ��汾��              */
        printk("ehdr->e_ident[EI_VERSION] != EV_CURRENT\n");
        return -1;
    }

    /*
     * Ŀ���ļ�����: ������ ���ض�λ�ļ� �� ����Ŀ���ļ�
     */
    if (ehdr->e_type != ET_REL && ehdr->e_type != ET_DYN) {
        printk("ehdr->e_type != ET_REL && ehdr->e_type != ET_DYN\n");
        return -1;
    }

    if (ehdr->e_machine != EM_ARM) {                                    /*  ��ϵ�ṹ����: ������ ARM    */
        printk("ehdr->e_machine != EM_ARM\n");
        return -1;
    }

    /*
     * ��֤�����ڳ����ײ�������صĶ���
     */
    if (ehdr->e_phoff != 0) {
        printk("ehdr->e_phoff != 0\n");
        return -1;
    }

    if (ehdr->e_phentsize != 0) {
        printk("ehdr->e_phentsize != 0\n");
        return -1;
    }

    if (ehdr->e_phnum != 0) {
        printk("ehdr->e_phnum != 0\n");
        return -1;
    }

    /*
     * ��֤���ڽ����ײ�������صĶ���
     */
    if (ehdr->e_shoff == 0) {
        printk("ehdr->e_shoff == 0\n");
        return -1;
    }

    if (ehdr->e_shentsize == 0) {
        printk("ehdr->e_shentsize == 0\n");
        return -1;
    }

    if (ehdr->e_shnum == 0) {
        printk("ehdr->e_shnum == 0\n");
        return -1;
    }

    return 0;
}
/*********************************************************************************************************
** Function name:           module_reloc
** Descriptions:            �ض�λģ��
** input parameters:        mod                 ģ��
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
static int module_reloc(module_t *mod)
{
    Elf32_Ehdr *ehdr;
    Elf32_Shdr *shdr;
    Elf32_Shdr **shdrs;
    Elf32_Shdr *shstrtab_shdr;
    char       *shstrtab;
    int         i;
    int         bss_idx;
    int         text_idx;

    ehdr = (Elf32_Ehdr *)mod->elf;                                      /*  ELF �ײ�                    */

    /*
     * �����ײ��е��ַ����Ľ����ײ�
     */
    shstrtab_shdr = (Elf32_Shdr *)(mod->elf + ehdr->e_shoff + ehdr->e_shstrndx * ehdr->e_shentsize);
    shstrtab      = (char *)mod->elf + shstrtab_shdr->sh_offset;        /*  �����ײ����ַ�����          */

    /*
     * ��װ�����н����ײ����ֲ�����
     */
    shdrs = kmalloc(ehdr->e_shnum * sizeof(Elf32_Shdr *), GFP_KERNEL);
    if (shdrs == NULL) {
        goto error0;
    }

    bss_idx  = SHN_UNDEF;
    text_idx = SHN_UNDEF;

    for (i = 0; i < ehdr->e_shnum; i++) {
        shdrs[i] = shdr = (Elf32_Shdr *)(mod->elf + ehdr->e_shoff + ehdr->e_shentsize * i);
        /*
         * .bss ���������� ELF �ļ���, ��Ҫ�������
         */
        if (strcmp(shdr->sh_name + shstrtab, ".bss") == 0) {
            bss_idx = i;
            shdr->sh_offset = (Elf32_Off)kzalloc(shdr->sh_size, GFP_KERNEL);
            if (shdr->sh_offset == 0) {
                goto error1;
            }
            /*
             * ˳��� .text ����Ҳ�ҳ�����
             */
        } else if (strcmp(shdr->sh_name + shstrtab, ".text") == 0) {
            text_idx = i;
        }
    }

    /*
     * ����������Ҫ�ض�λ�Ľ���
     */
    for (i = 0, shdr = shdrs[0]; i < ehdr->e_shnum; i++, shdr = shdrs[i]) {

        Elf32_Shdr *target_shdr;
        Elf32_Shdr *symtab_shdr;
        Elf32_Shdr *strtab_shdr;
        char       *strtab;
        uint8_t    *symtab;
        uint8_t    *target;

        if (shdr->sh_type == SHT_REL || shdr->sh_type == SHT_RELA) {    /*  �ý�����Ҫ�ض�λ            */

            if (shdr->sh_link == SHN_UNDEF) {
                goto error1;
            }
            symtab_shdr = shdrs[shdr->sh_link];                         /*  ���ű������ײ�              */

            if (shdr->sh_info == SHN_UNDEF) {
                goto error1;
            }
            target_shdr = shdrs[shdr->sh_info];                         /*  Ŀ������Ľ����ײ�          */

            if (symtab_shdr->sh_link == SHN_UNDEF) {
                goto error1;
            }
            strtab_shdr = shdrs[symtab_shdr->sh_link];                  /*  �ַ����������ײ�            */

            symtab      = mod->elf + symtab_shdr->sh_offset;            /*  ���ű�                      */
            target      = mod->elf + target_shdr->sh_offset;            /*  Ŀ�����                    */
            strtab      = (char *)mod->elf + strtab_shdr->sh_offset;    /*  �ַ�����                    */
        }

        if (shdr->sh_type == SHT_REL) {                                 /*  �ض�λ����Ϊ REL            */
            Elf32_Rel *rel;
            int        j;

            /*
             * �������е� rel ��Ŀ
             */
            for (j = 0, rel = (Elf32_Rel *)(mod->elf + shdr->sh_offset);
                 j < shdr->sh_size / shdr->sh_entsize;
                 j++,   rel = (Elf32_Rel *)((char *)rel + shdr->sh_entsize)) {

                int sym_idx;

                sym_idx = ELF32_R_SYM(rel->r_info);                     /*  ��� rel ��Ŀ�ķ�������     */

                if (STN_UNDEF != sym_idx) {
                    Elf32_Sym  *sym;
                    Elf32_Addr  addr;
                    int         ret;
                                                                        /*  ��� rel ��Ŀ�ķ���         */
                    sym = (Elf32_Sym *)(symtab + sym_idx * symtab_shdr->sh_entsize);

                    if (sym->st_shndx != SHN_UNDEF) {                   /*  ������������ ELF �ļ���   */
                        /*
                         * ������������ ELF �ļ����ַ
                         */
                        if (sym->st_shndx == bss_idx) {                 /*  ������������ .bss ����    */
                            addr = (Elf32_Addr)(shdrs[bss_idx]->sh_offset + sym->st_value);
                        } else {
                            addr = (Elf32_Addr)(mod->elf +
                                    shdrs[sym->st_shndx]->sh_offset + sym->st_value);
                        }

                        /*
                         * �Ըõ� rel ��Ŀ�����ض�λ
                         */
                        ret = arm_reloc_rel(rel, addr, target);
                        if (ret < 0) {
                            goto error1;
                        }
                    } else {
                        /*
                         * ��ϵͳ������������
                         */
                        extern uint32_t symbol_lookup(const char *name, uint32_t type);
                        addr = symbol_lookup(sym->st_name + strtab,
                                ELF32_ST_TYPE(sym->st_info) == STT_FUNC ? SYMBOL_TEXT : SYMBOL_DATA);
                        if (addr == 0) {
                            printk("symbol %s no found\n", sym->st_name + strtab);
                            goto error1;
                        } else {
                            /*
                             * �Ըõ� rel ��Ŀ�����ض�λ
                             */
                            ret = arm_reloc_rel(rel, addr, target);
                            if (ret < 0) {
                                goto error1;
                            }
                        }
                    }
                } else {
                    goto error1;
                }
            }
        } else if (shdr->sh_type == SHT_RELA) {
            goto error1;
        }
    }

    mod->shdrs    = shdrs;
    mod->bss_idx  = bss_idx;
    mod->text_idx = text_idx;

    //dcache_clean_invalidate(mod->elf, mod->size);

    return 0;

    error1:
    kfree(shdrs);

    error0:
    return -1;
}
/*********************************************************************************************************
** Function name:           module_exec
** Descriptions:            ִ��ģ��
** input parameters:        mod                 ģ��
**                          func_name           ������
**                          argc                ��������
**                          argv                ��������
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
static int module_exec(module_t *mod, const char *func_name, int argc, char **argv)
{
    int         i;
    Elf32_Ehdr *ehdr;
    Elf32_Shdr *shdr;
    int (*func)(int argc, char **argv);

    ehdr = (Elf32_Ehdr *)mod->elf;                                      /*  ELF �ײ�                    */

    /*
     * ��ģ��ķ��ű�������������
     */
    for (i = 0, shdr = mod->shdrs[0]; i < ehdr->e_shnum; i++, shdr = mod->shdrs[i]) {

        if (shdr->sh_type == SHT_SYMTAB) {                              /*  �����ײ�����: ���ű�        */
            Elf32_Shdr *symtab_shdr;
            Elf32_Shdr *strtab_shdr;
            char       *strtab;
            uint8_t    *symtab;
            Elf32_Sym  *sym;
            int         j;

            symtab_shdr = shdr;                                         /*  ���ű������ײ�              */
            symtab      = mod->elf + symtab_shdr->sh_offset;            /*  ���ű� ��                   */

            if (symtab_shdr->sh_link == SHN_UNDEF) {
                continue;
            }
            strtab_shdr = mod->shdrs[symtab_shdr->sh_link];             /*  �ַ����������ײ�            */
            strtab      = (char *)mod->elf + strtab_shdr->sh_offset;    /*  �ַ�����                    */

            for (j = 0,
                 sym = (Elf32_Sym *)symtab;
                 j < symtab_shdr->sh_size / symtab_shdr->sh_entsize;
                 j++, sym = (Elf32_Sym *)((char *)sym + symtab_shdr->sh_entsize)) {

                if (sym->st_shndx == mod->text_idx &&                   /*  �÷����� TEXT ��            */
                    ELF32_ST_TYPE(sym->st_info) == STT_FUNC) {          /*  �÷����Ǻ�������            */

                    if (strcmp(sym->st_name + strtab, func_name) == 0) {/*  �÷�������ƥ�䡡            */

                        func = (int (*)(int, char **))(mod->elf +       /*  �÷����� ELF �ļ����λ��   */
                                mod->shdrs[mod->text_idx]->sh_offset + sym->st_value);

                        return func(argc, argv);                        /*  ִ�иú���                  */
                    }
                }
            }
        }
    }
    return -1;
}
/*********************************************************************************************************
** Function name:           module_load
** Descriptions:            ���� ELF �ļ�
** input parameters:        path                ELF �ļ�·��
**                          argc                ��������
**                          argv                ��������
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
int module_load(const char *path, int argc, char **argv)
{
    int          fd;
    int          ret;
    int          len;
    uint8_t     *buf;
    struct stat  st;
    module_t    *mod;

    if (path == NULL) {
        printk("%s: path = NULL\n", __func__);
        return -1;
    }

    ret = access(path, R_OK);                                           /*  ��������ļ��ܷ����        */
    if (ret < 0) {
        printk("%s: failed to access %s\n", __func__, path);
        return -1;
    }

    fd = open(path, O_RDONLY);                                          /*  ���ļ�                    */
    if (fd < 0) {
        printk("%s: failed to open %s\n", __func__, path);
        return -1;
    }

    ret = fstat(fd, &st);                                               /*  ����ļ��Ĵ�С              */
    if (ret < 0 || st.st_size == 0) {
        printk("%s: failed to stat %s or %s is empty\n", __func__, path, path);
        return -1;
    }

    mod = module_alloc(path, st.st_size);                               /*  ����ģ��                    */
    if (mod == NULL) {
        printk("%s: failed to alloc module %s, %d byte\n", __func__, path, st.st_size);
        close(fd);
        return -1;
    }

    len = 0;                                                            /*  ���ļ������ݶ���������      */
    buf = mod->elf;
    while (st.st_size - len > 0) {
        ret = read(fd, buf, st.st_size - len);
        if (ret < 0) {
            printk("%s: failed to read %s %d byte at %d\n", __func__, path, st.st_size - len, len);
            module_free(mod);
            close(fd);
            return -1;
        }
        buf += ret;
        len += ret;
    }

    close(fd);                                                          /*  �ر��ļ�                    */

    ret = module_probe(mod);                                            /*  ̽��ģ��                    */
    if (ret < 0) {
        printk("%s: %s is not a module file\n", __func__, path);
        module_free(mod);
        return -1;
    }

    ret = module_reloc(mod);                                            /*  �ض�λģ��                  */
    if (ret < 0) {
        printk("%s: failed to reloc %s\n", __func__, path);
        module_free(mod);
        return -1;
    }

    mutex_lock(&mod_mgr_lock, 0);

    if (module_lookup_by_key(mod->key) != NULL) {                       /*  ����ģ���Ƿ��Ѿ�����      ��*/
        mutex_unlock(&mod_mgr_lock);
        module_free(mod);
        return -1;
    }

    module_install(mod);                                                /*  ��װģ��                    */

    mutex_unlock(&mod_mgr_lock);

    ret = module_exec(mod, "module_init", argc, argv);                  /*  ִ��ģ���ʼ������          */
    if (ret < 0) {
        printk("%s: failed to exec %s module_init\n", __func__, path);
        module_remove(mod);
        module_free(mod);
        return -1;
    }

    return 0;
}
/*********************************************************************************************************
** Function name:           module_unload
** Descriptions:            ж��ģ��
** input parameters:        module              ģ��
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
int module_unload(const char *path)
{
    module_t *mod;
    int ret;

    if (path == NULL) {
        printk("%s: path = NULL\n", __func__);
        return -1;
    }

    mod = module_lookup(path);
    if (mod == NULL) {
        printk("%s: module %s no found\n", __func__, path);
        return -1;
    }

    ret = module_exec(mod, "module_exit", 0, NULL);
    if (ret < 0) {
        printk("%s: failed to exec %s module_exit\n", __func__, path);
        return -1;
    }

    if (atomic_read(&mod->ref) != 0) {
        printk("%s: module %s is busy\n", __func__, path);
        return -1;
    }

    module_remove(mod);

    module_free(mod);

    return 0;
}
/*********************************************************************************************************
** END FILE
*********************************************************************************************************/